// ============================================================
// Trenzin — ESP32 + LCD 1602A (Wi-Fi WebSocket)
// ============================================================
// Wiring (4-bit parallel mode for ESP32 - 3.3V Logic):
//   LCD RS  → GPIO 19   LCD Enable → GPIO 23
//   LCD D4  → GPIO 18   LCD D5     → GPIO 17
//   LCD D6  → GPIO 16   LCD D7     → GPIO 15
//   LCD R/W → GND       (CRITICAL! Must be GND to protect ESP32)
//   LCD VDD → 5V (VIN)  LCD VSS    → GND
//   LCD V0  → GPIO 13   (Contrast control via PWM)
//
// Buttons (INPUT_PULLUP):
//   BTN_PLAY_PIN → GPIO 26 (Play/Pause Pomodoro)
//   BTN_STOP_PIN → GPIO 25 (Stop Pomodoro)
//   BTN_EXPR_PIN → GPIO 27 (Cycle Expressions)
//   BTN_SETUP_PIN→ GPIO 32 (Settings Menu)
//   POT_PIN      → GPIO 34 (Potentiometer for setup, 3.3V max)
// ============================================================

#include "web_assets.h"
#include <ESPmDNS.h>
#include <LiquidCrystal.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <time.h>

Preferences preferences;

// ── Pin Configuration ──────────────────────────────────────
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);
#define BTN_PLAY_PIN 26
#define BTN_STOP_PIN 25
#define BTN_EXPR_PIN 27
#define BTN_SETUP_PIN 32
#define POT_PIN 34

// ── Wi-Fi Configuration ────────────────────────────────────
// As credenciais agora são gerenciadas pelo WiFiManager

WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);

// ── States ─────────────────────────────────────────────────
enum Expression {
  EXPR_IDLE,
  EXPR_HAPPY,
  EXPR_SAD,
  EXPR_ANGRY,
  EXPR_FOCUS,
  EXPR_SLEEP,
  EXPR_LOVE,
  EXPR_SURPRISED,
  EXPR_STAR,
  EXPR_DIZZY
};

enum TimerState {
  TIMER_OFF,
  TIMER_FOCUS,
  TIMER_FOCUS_PAUSED,
  TIMER_BREAK,
  TIMER_BREAK_PAUSED,
  TIMER_DONE
};

enum SetupState {
  SETUP_OFF,
  SETUP_SELECT_MENU,
  SETUP_POMO_FOCUS,
  SETUP_POMO_SHORT_BREAK,
  SETUP_POMO_LONG_BREAK,
  SETUP_ALARM_SELECT,
  SETUP_ALARM_HOUR,
  SETUP_ALARM_MINUTE,
  SETUP_ALARM_STATE
};

// ── Current State ──────────────────────────────────────────
Expression currentExpr = EXPR_IDLE;
TimerState timerState = TIMER_OFF;
unsigned long timerSecondsRemaining = 0;
unsigned long lastTimerTick = 0;
unsigned long exprTimeout = 0; // Auto-return to idle
unsigned long lastBlinkTime = 0;
unsigned long nextBlinkInterval = 3000; // ms between blinks
bool isBlinking = false;
unsigned long blinkStart = 0;
const unsigned long BLINK_DURATION = 180; // ms

// Timer durations (can be modified via Potentiometer)
unsigned long focusDuration = 25UL * 60;     // default 25 min
unsigned long shortBreakDuration = 5UL * 60; // default 5 min
unsigned long longBreakDuration = 15UL * 60; // default 15 min

SetupState currentSetupState = SETUP_OFF;

// ── Temporary Message ──────────────────────────────────────
String tempMessage = "";
unsigned long tempMsgTimeout = 0;

// ── Alarm State ────────────────────────────────────────────
#define MAX_ALARMS 10
struct AlarmConfig {
  bool enabled;
  int hour;
  int minute;
  bool triggeredToday;
};
AlarmConfig alarms[MAX_ALARMS];
int currentDay = -1;

// Variáveis para configuração de alarmes via potenciômetro
int setupAlarmId = 0;
int setupAlarmHour = 0;
int setupAlarmMinute = 0;
bool setupAlarmEnabled = false;
int existingAlarmIds[MAX_ALARMS];
int existingAlarmCount = 0;

// Variáveis para configuração de pomodoro via potenciômetro
unsigned long setupFocusDuration = 0;
unsigned long setupShortBreakDuration = 0;
unsigned long setupLongBreakDuration = 0;

void saveAlarm(int id) {
  if (id >= 0 && id < MAX_ALARMS) {
    String keyEn = "alm_en_" + String(id);
    String keyHr = "alm_hr_" + String(id);
    String keyMn = "alm_mn_" + String(id);
    preferences.putBool(keyEn.c_str(), alarms[id].enabled);
    preferences.putInt(keyHr.c_str(), alarms[id].hour);
    preferences.putInt(keyMn.c_str(), alarms[id].minute);
  }
}

void loadAlarms() {
  for (int i = 0; i < MAX_ALARMS; i++) {
    String keyEn = "alm_en_" + String(i);
    String keyHr = "alm_hr_" + String(i);
    String keyMn = "alm_mn_" + String(i);
    alarms[i].enabled = preferences.getBool(keyEn.c_str(), false);
    alarms[i].hour = preferences.getInt(keyHr.c_str(), 0);
    alarms[i].minute = preferences.getInt(keyMn.c_str(), 0);
    alarms[i].triggeredToday = false;
  }
}

// ── Custom Characters (5×8 pixels) ────────────────────────
// Slot 0: Left eye open
byte eyeLeftOpen[8] = {B00000, B01110, B10001, B10011,
                       B10011, B10001, B01110, B00000};
// Slot 1: Right eye open
byte eyeRightOpen[8] = {B00000, B01110, B10001, B11001,
                        B11001, B10001, B01110, B00000};
// Slot 2: Eye closed (blink — same for both)
byte eyeClosed[8] = {B00000, B00000, B00000, B01110,
                     B10001, B00000, B00000, B00000};
// Slot 3: Happy left eye (^)
byte eyeHappyL[8] = {B00000, B00000, B10001, B01010,
                     B00100, B00000, B00000, B00000};
// Slot 4: Happy right eye (^)
byte eyeHappyR[8] = {B00000, B00000, B10001, B01010,
                     B00100, B00000, B00000, B00000};
// Slot 5: Sad eye
byte eyeSad[8] = {B00000, B00000, B00100, B01010,
                  B10001, B10001, B01110, B00000};
// Slot 6: Angry eye
byte eyeAngry[8] = {B10000, B01000, B01110, B10001,
                    B10011, B10001, B01110, B00000};
// Slot 7: Focus eye (determined squint)
byte eyeFocus[8] = {B00000, B00000, B01110, B10001,
                    B10011, B01110, B00000, B00000};
// Slot 8: Love eye (Heart)
byte eyeLove[8] = {B00000, B01010, B11111, B11111,
                   B01110, B00100, B00000, B00000};
// Slot 9: Surprised eye
byte eyeSurprised[8] = {B00000, B01110, B10001, B10101,
                        B10101, B10001, B01110, B00000};
// Slot 10: Star eye
byte eyeStar[8] = {B00100, B10101, B01110, B11111,
                   B01110, B10101, B00100, B00000};
// Slot 11: Dizzy eye (X shape)
byte eyeDizzy[8] = {B00000, B10001, B01010, B00100,
                    B01010, B10001, B00000, B00000};

// ── Redraw Helpers ─────────────────────────────────────────
Expression lastDrawnExpr = (Expression)255;
bool lastDrawnBlink = false;
TimerState lastDrawnTimerState = (TimerState)255;
unsigned long lastDrawnSeconds = 999999;
String lastDrawnMsg = "";

// ── Connection Tracking ────────────────────────────────────
int connectedClients = 0;
bool lastConnectionState = false;

// ── Send to all WebSocket Clients ──────────────────────────
void sendToClients(String msg) {
  webSocket.broadcastTXT(msg);
  Serial.println("TX: " + msg);
}

void sendAlarmConfig(int id) {
  if (id >= 0 && id < MAX_ALARMS) {
    String msg = "CFG:ALM:" + String(id) + ":" +
                 String(alarms[id].enabled ? "1" : "0") + ":" +
                 String(alarms[id].hour) + ":" + String(alarms[id].minute);
    sendToClients(msg);
  }
}

// ── Physical Buttons ───────────────────────────────────────
unsigned long lastBtnPlayTime = 0;
unsigned long lastBtnStopTime = 0;
unsigned long lastBtnExprTime = 0;
unsigned long lastBtnSetupTime = 0;
const unsigned long BTN_COOLDOWN = 300; // 300ms between presses

void updateExistingAlarms() {
  existingAlarmCount = 0;

  // 1. Coleta todos os alarmes que já existem (configurados)
  for (int i = 0; i < MAX_ALARMS; i++) {
    if (alarms[i].enabled || alarms[i].hour != 0 || alarms[i].minute != 0) {
      existingAlarmIds[existingAlarmCount] = i;
      existingAlarmCount++;
    }
  }

  // 2. Encontra o primeiro alarme não utilizado (se houver espaço) para poder
  // criá-lo
  if (existingAlarmCount < MAX_ALARMS) {
    for (int i = 0; i < MAX_ALARMS; i++) {
      bool alreadyInList = false;
      for (int j = 0; j < existingAlarmCount; j++) {
        if (existingAlarmIds[j] == i) {
          alreadyInList = true;
          break;
        }
      }
      if (!alreadyInList) {
        existingAlarmIds[existingAlarmCount] = i;
        existingAlarmCount++;
        break;
      }
    }
  }

  // 3. Ordena os IDs para navegação sequencial no potenciômetro
  for (int i = 0; i < existingAlarmCount - 1; i++) {
    for (int j = 0; j < existingAlarmCount - i - 1; j++) {
      if (existingAlarmIds[j] > existingAlarmIds[j + 1]) {
        int temp = existingAlarmIds[j];
        existingAlarmIds[j] = existingAlarmIds[j + 1];
        existingAlarmIds[j + 1] = temp;
      }
    }
  }
}

void checkSetupMenu() {

  if (currentSetupState == SETUP_OFF)
    return;

  unsigned long now = millis();
  static unsigned long lastPotRead = 0;

  // Read potentiometer every 100ms for smooth UI updates
  if (now - lastPotRead > 100) {
    lastPotRead = now;
    int potValue = analogRead(POT_PIN);
    char buf[17];

    switch (currentSetupState) {
    case SETUP_SELECT_MENU: {
      lcd.setCursor(0, 0);
      lcd.print("[Config] Menu   ");
      lcd.setCursor(0, 1);
      if (potValue < 2048) {
        lcd.print("> 1. Pomodoro   ");
      } else {
        lcd.print("> 2. Alarmes    ");
      }
      break;
    }
    case SETUP_POMO_FOCUS: {
      unsigned int minutes = constrain(map(potValue, 0, 4095, 1, 60), 1, 60);
      setupFocusDuration = minutes * 60;
      lcd.setCursor(0, 0);
      lcd.print("[Config] Foco   ");
      lcd.setCursor(0, 1);
      snprintf(buf, sizeof(buf), "Tempo: %02d min    ", minutes);
      lcd.print(buf);
      break;
    }
    case SETUP_POMO_SHORT_BREAK: {
      unsigned int minutes = constrain(map(potValue, 0, 4095, 1, 30), 1, 30);
      setupShortBreakDuration = minutes * 60;
      lcd.setCursor(0, 0);
      lcd.print("[Config] Pausa C");
      lcd.setCursor(0, 1);
      snprintf(buf, sizeof(buf), "Tempo: %02d min    ", minutes);
      lcd.print(buf);
      break;
    }
    case SETUP_POMO_LONG_BREAK: {
      unsigned int minutes = constrain(map(potValue, 0, 4095, 1, 45), 1, 45);
      setupLongBreakDuration = minutes * 60;
      lcd.setCursor(0, 0);
      lcd.print("[Config] Pausa L");
      lcd.setCursor(0, 1);
      snprintf(buf, sizeof(buf), "Tempo: %02d min    ", minutes);
      lcd.print(buf);
      break;
    }
    case SETUP_ALARM_SELECT: {
      updateExistingAlarms();
      int mappedIndex =
          constrain(map(potValue, 0, 4095, 0, existingAlarmCount - 1), 0,
                    existingAlarmCount - 1);
      int selectedAlarmId = existingAlarmIds[mappedIndex];
      lcd.setCursor(0, 0);
      lcd.print("[Config] Alarme ");
      lcd.setCursor(0, 1);
      snprintf(buf, sizeof(buf), "Sel. Alarme: %02d ", selectedAlarmId + 1);
      lcd.print(buf);
      break;
    }
    case SETUP_ALARM_HOUR: {
      unsigned int hourVal = constrain(map(potValue, 0, 4095, 0, 23), 0, 23);
      setupAlarmHour = hourVal;
      lcd.setCursor(0, 0);
      snprintf(buf, sizeof(buf), "Alarme %02d: Hora  ", setupAlarmId + 1);
      lcd.print(buf);
      lcd.setCursor(0, 1);
      snprintf(buf, sizeof(buf), "Hora: %02d         ", hourVal);
      lcd.print(buf);
      break;
    }
    case SETUP_ALARM_MINUTE: {
      unsigned int minVal = constrain(map(potValue, 0, 4095, 0, 59), 0, 59);
      setupAlarmMinute = minVal;
      lcd.setCursor(0, 0);
      snprintf(buf, sizeof(buf), "Alarme %02d: Minuto", setupAlarmId + 1);
      lcd.print(buf);
      lcd.setCursor(0, 1);
      snprintf(buf, sizeof(buf), "Minuto: %02d       ", minVal);
      lcd.print(buf);
      break;
    }
    case SETUP_ALARM_STATE: {
      bool stateVal = (potValue >= 2048);
      setupAlarmEnabled = stateVal;
      lcd.setCursor(0, 0);
      snprintf(buf, sizeof(buf), "Alarme %02d: Status", setupAlarmId + 1);
      lcd.print(buf);
      lcd.setCursor(0, 1);
      if (stateVal) {
        lcd.print("Status: LIGADO  ");
      } else {
        lcd.print("Status: DESLIGAD");
      }
      break;
    }
    default:
      break;
    }
  }
}

void showSavedMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("     Salvo!     ");
  delay(1000);
  lcd.clear();
  lastDrawnExpr = (Expression)255;
  lastDrawnTimerState = (TimerState)255;
}

void handleSetupCancel() {
  currentSetupState = SETUP_OFF;
  lcd.clear();
  lastDrawnExpr = (Expression)255;
  lastDrawnTimerState = (TimerState)255;
  Serial.println("Configuracao cancelada sem salvar.");
}

void handleSetupBack() {
  switch (currentSetupState) {
  case SETUP_POMO_FOCUS:
    currentSetupState = SETUP_SELECT_MENU;
    break;
  case SETUP_POMO_SHORT_BREAK:
    currentSetupState = SETUP_POMO_FOCUS;
    break;
  case SETUP_POMO_LONG_BREAK:
    currentSetupState = SETUP_POMO_SHORT_BREAK;
    break;
  case SETUP_ALARM_SELECT:
    currentSetupState = SETUP_SELECT_MENU;
    break;
  case SETUP_ALARM_HOUR:
    currentSetupState = SETUP_ALARM_SELECT;
    break;
  case SETUP_ALARM_MINUTE:
    currentSetupState = SETUP_ALARM_HOUR;
    break;
  case SETUP_ALARM_STATE:
    currentSetupState = SETUP_ALARM_MINUTE;
    break;
  default:
    break;
  }
  lcd.clear();
}

void handleSetupAdvance(bool isButton3) {
  int potValue = analogRead(POT_PIN);

  if (currentSetupState == SETUP_SELECT_MENU) {
    if (potValue < 2048) {
      setupFocusDuration = focusDuration;
      setupShortBreakDuration = shortBreakDuration;
      setupLongBreakDuration = longBreakDuration;
      currentSetupState = SETUP_POMO_FOCUS;
    } else {
      currentSetupState = SETUP_ALARM_SELECT;
    }
    lcd.clear();
  } else if (currentSetupState == SETUP_POMO_FOCUS) {
    setupFocusDuration = constrain(map(potValue, 0, 4095, 1, 60), 1, 60) * 60;
    currentSetupState = SETUP_POMO_SHORT_BREAK;
    lcd.clear();
  } else if (currentSetupState == SETUP_POMO_SHORT_BREAK) {
    setupShortBreakDuration =
        constrain(map(potValue, 0, 4095, 1, 30), 1, 30) * 60;
    currentSetupState = SETUP_POMO_LONG_BREAK;
    lcd.clear();
  } else if (currentSetupState == SETUP_POMO_LONG_BREAK) {
    if (isButton3) {
      // Button 3 (Next) does nothing on the last step because there's no next
      // config
      return;
    }
    setupLongBreakDuration =
        constrain(map(potValue, 0, 4095, 1, 45), 1, 45) * 60;

    // Save to flash
    focusDuration = setupFocusDuration;
    shortBreakDuration = setupShortBreakDuration;
    longBreakDuration = setupLongBreakDuration;

    preferences.putUInt("focus", focusDuration / 60);
    preferences.putUInt("sbreak", shortBreakDuration / 60);
    preferences.putUInt("lbreak", longBreakDuration / 60);
    Serial.println("Configuracoes de Pomodoro salvas na Flash!");

    // Broadcast new settings to all connected Web UIs
    String cfgMsg = "CFG:POMO:" + String(focusDuration / 60) + ":" +
                    String(shortBreakDuration / 60) + ":" +
                    String(longBreakDuration / 60);
    webSocket.broadcastTXT(cfgMsg);

    currentSetupState = SETUP_OFF;
    showSavedMessage();
  } else if (currentSetupState == SETUP_ALARM_SELECT) {
    updateExistingAlarms();
    int mappedIndex =
        constrain(map(potValue, 0, 4095, 0, existingAlarmCount - 1), 0,
                  existingAlarmCount - 1);
    setupAlarmId = existingAlarmIds[mappedIndex];
    // Initialize temp editing variables from current alarm config
    setupAlarmHour = alarms[setupAlarmId].hour;
    setupAlarmMinute = alarms[setupAlarmId].minute;
    setupAlarmEnabled = alarms[setupAlarmId].enabled;

    currentSetupState = SETUP_ALARM_HOUR;
    lcd.clear();
  } else if (currentSetupState == SETUP_ALARM_HOUR) {
    setupAlarmHour = constrain(map(potValue, 0, 4095, 0, 23), 0, 23);
    currentSetupState = SETUP_ALARM_MINUTE;
    lcd.clear();
  } else if (currentSetupState == SETUP_ALARM_MINUTE) {
    setupAlarmMinute = constrain(map(potValue, 0, 4095, 0, 59), 0, 59);
    currentSetupState = SETUP_ALARM_STATE;
    lcd.clear();
  } else if (currentSetupState == SETUP_ALARM_STATE) {
    if (isButton3) {
      // Button 3 (Next) does nothing on the last step because there's no next
      // config
      return;
    }
    setupAlarmEnabled = (potValue >= 2048);

    // Apply changes
    alarms[setupAlarmId].hour = setupAlarmHour;
    alarms[setupAlarmId].minute = setupAlarmMinute;
    alarms[setupAlarmId].enabled = setupAlarmEnabled;
    alarms[setupAlarmId].triggeredToday = false;

    // Save and Sync
    saveAlarm(setupAlarmId);
    sendAlarmConfig(setupAlarmId);

    currentSetupState = SETUP_OFF;
    showSavedMessage();
  }
}

void checkButtons() {
  unsigned long now = millis();

  // Intercept button functions if setup mode is active
  if (currentSetupState != SETUP_OFF) {
    // Button 4 (GPIO 32 / BTN_SETUP) - Advance/Confirm/Save
    if (digitalRead(BTN_SETUP_PIN) == LOW) {
      if (now - lastBtnSetupTime > BTN_COOLDOWN) {
        lastBtnSetupTime = now;
        Serial.println("BTN: SETUP pressionado no menu (GPIO 32)");
        handleSetupAdvance(false);
      }
    }
    // Button 2 (GPIO 26 / BTN_PLAY) - Back
    if (digitalRead(BTN_PLAY_PIN) == LOW) {
      if (now - lastBtnPlayTime > BTN_COOLDOWN) {
        lastBtnPlayTime = now;
        Serial.println("BTN: BACK pressionado no menu (GPIO 26)");
        handleSetupBack();
      }
    }
    // Button 3 (GPIO 25 / BTN_STOP) - Next
    if (digitalRead(BTN_STOP_PIN) == LOW) {
      if (now - lastBtnStopTime > BTN_COOLDOWN) {
        lastBtnStopTime = now;
        Serial.println("BTN: NEXT pressionado no menu (GPIO 25)");
        handleSetupAdvance(true);
      }
    }
    // Button 1 (GPIO 27 / BTN_EXPR) - Cancel without saving
    if (digitalRead(BTN_EXPR_PIN) == LOW) {
      if (now - lastBtnExprTime > BTN_COOLDOWN) {
        lastBtnExprTime = now;
        Serial.println("BTN: CANCEL pressionado no menu (GPIO 27)");
        handleSetupCancel();
      }
    }
    return; // Block other button operations while in setup mode
  }

  // Normal mode button checks (when setup mode is inactive)
  // Setup (GPIO 32)
  if (digitalRead(BTN_SETUP_PIN) == LOW) {
    if (now - lastBtnSetupTime > BTN_COOLDOWN) {
      lastBtnSetupTime = now;
      Serial.println("BTN: SETUP pressionado (GPIO 32)");
      currentSetupState = SETUP_SELECT_MENU;
      lcd.clear();
    }
  }

  // Play/Pause (GPIO 26)
  if (digitalRead(BTN_PLAY_PIN) == LOW) {
    if (now - lastBtnPlayTime > BTN_COOLDOWN) {
      lastBtnPlayTime = now;
      Serial.println("BTN: PLAY/PAUSE pressionado (GPIO 26)");
      if (timerState == TIMER_FOCUS) {
        timerState = TIMER_FOCUS_PAUSED;
      } else if (timerState == TIMER_BREAK) {
        timerState = TIMER_BREAK_PAUSED;
      } else if (timerState == TIMER_FOCUS_PAUSED) {
        timerState = TIMER_FOCUS;
        lastTimerTick = now;
        setExpression(EXPR_FOCUS);
      } else if (timerState == TIMER_BREAK_PAUSED) {
        timerState = TIMER_BREAK;
        lastTimerTick = now;
      } else {
        timerState = TIMER_FOCUS;
        timerSecondsRemaining = focusDuration;
        lastTimerTick = now;
        setExpression(EXPR_FOCUS);
      }
      sendTimerState();
      drawStatusLine();
    }
  }

  // Stop (GPIO 25)
  if (digitalRead(BTN_STOP_PIN) == LOW) {
    if (now - lastBtnStopTime > BTN_COOLDOWN) {
      lastBtnStopTime = now;
      Serial.println("BTN: STOP pressionado (GPIO 25)");
      timerState = TIMER_OFF;
      timerSecondsRemaining = 0;
      setExpression(EXPR_IDLE);
      sendTimerState();
      drawStatusLine();
    }
  }

  // Expression Cycle (GPIO 27)
  if (digitalRead(BTN_EXPR_PIN) == LOW) {
    if (now - lastBtnExprTime > BTN_COOLDOWN) {
      lastBtnExprTime = now;
      Serial.println("BTN: EXPR pressionado (GPIO 27)");
      int nextExpr = (int)currentExpr + 1;
      if (nextExpr > EXPR_DIZZY)
        nextExpr = EXPR_IDLE;
      setExpression((Expression)nextExpr);
    }
  }
}

// ── Setup ──────────────────────────────────────────────────
void configModeCallback(WiFiManager *myWiFiManager) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conecte no WiFi:");
  lcd.setCursor(0, 1);
  lcd.print("Trenzin-Setup   ");
}

void setup() {
  Serial.begin(115200);

  // Carregar preferências salvas
  preferences.begin("trenzin", false);
  focusDuration = preferences.getUInt("focus", 25) * 60;
  shortBreakDuration = preferences.getUInt("sbreak", 5) * 60;
  longBreakDuration = preferences.getUInt("lbreak", 15) * 60;
  loadAlarms();

  pinMode(BTN_PLAY_PIN, INPUT_PULLUP);
  pinMode(BTN_STOP_PIN, INPUT_PULLUP);
  pinMode(BTN_EXPR_PIN, INPUT_PULLUP);
  pinMode(BTN_SETUP_PIN, INPUT_PULLUP);
  // POT_PIN does not need pinMode for analogRead on ESP32

  // Truque mágico do Contraste (PWM) no Pino 13
  pinMode(13, OUTPUT);
  // O valor vai de 0 (máximo escuro/GND) a 255 (apagado).
  // 60 costuma ser o ponto doce perfeito! Mude se precisar.
  analogWrite(13, 75);

  lcd.begin(16, 2);

  loadExpressionChars(EXPR_IDLE);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Trenzin OS");
  lcd.setCursor(0, 1);
  lcd.print("Conectando...");

  // Connect to Wi-Fi using WiFiManager
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  // Opcional: Se precisar resetar as configurações de Wi-Fi salvas para testar
  // o portal cativo, descomente a linha abaixo.
  // wifiManager.resetSettings();

  // Tenta conectar nas redes conhecidas.
  // Se falhar ou não houver redes salvas, ele sobe um Access Point chamado
  // "Trenzin-Setup"
  if (!wifiManager.autoConnect("Trenzin-Setup")) {
    Serial.println("Falha ao conectar no Wi-Fi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Falha de Wi-Fi!");
    delay(3000);
    ESP.restart(); // Reinicia o ESP para tentar novamente
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("trenzin")) {
    Serial.println("MDNS responder started");
  }

  // Sync time with NTP (UTC-3 for Brazil)
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  // Show IP/Hostname on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wi-Fi OK!");
  lcd.setCursor(0, 1);
  lcd.print("trenzin.local");
  delay(5000); // Give user time to read

  // Start WebSocket Server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Setup Web Server Routes (com anti-cache)
  server.on("/", []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.send(200, "text/html", WEB_HTML);
  });
  server.on("/style.css", []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.send(200, "text/css", WEB_CSS);
  });
  server.on("/app.js", []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.send(200, "application/javascript", WEB_JS);
  });
  server.on("/manifest.json", []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.send(200, "application/json", WEB_MANIFEST);
  });
  server.on("/sw.js", []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.send(200, "application/javascript", WEB_SW);
  });
  server.on("/icon.svg", []() {
    server.sendHeader("Cache-Control", "max-age=604800, public");
    server.send(200, "image/svg+xml", WEB_ICON);
  });
  server.on("/icon-192.png", []() {
    server.sendHeader("Cache-Control", "max-age=604800, public");
    server.send_P(200, "image/png", (const char *)WEB_ICON_PNG,
                  WEB_ICON_PNG_LEN);
  });
  server.begin();

  // Initialize UI
  lcd.clear();
  drawFace();
  drawStatusLine();

  lastBlinkTime = millis();
  nextBlinkInterval = random(2500, 5000);
}

// ── WebSocket Event Handler ────────────────────────────────
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload,
                    size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    if (connectedClients > 0)
      connectedClients--;

    // Show temporary disconnection message on LCD
    tempMessage = "App Desconectado";
    tempMsgTimeout = millis() + 3000;
    lastDrawnMsg = "";
    drawStatusLine();
    break;
  case WStype_CONNECTED: {
    connectedClients++;
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2],
                  ip[3]);
    // Send initial state to the new client
    sendToClients("ACK:BOOT");
    sendToClients("STATE:IDLE");

    // Send current settings to the new client
    String cfgMsg = "CFG:POMO:" + String(focusDuration / 60) + ":" +
                    String(shortBreakDuration / 60) + ":" +
                    String(longBreakDuration / 60);
    webSocket.broadcastTXT(cfgMsg);

    for (int i = 0; i < MAX_ALARMS; i++) {
      sendAlarmConfig(i);
    }

    sendTimerState();

    // Show temporary connection message on LCD
    tempMessage = " App Conectado! ";
    tempMsgTimeout = millis() + 3000;
    lastDrawnMsg = "";
    drawStatusLine();
  } break;
  case WStype_TEXT: {
    String cmd = "";
    for (int i = 0; i < length; i++) {
      cmd += (char)payload[i];
    }
    Serial.printf("[%u] RX: %s\n", num, cmd.c_str());
    processCommand(cmd);
  } break;
  }
}

// ── Main Loop ──────────────────────────────────────────────
void loop() {
  webSocket.loop();
  server.handleClient();
  checkButtons();

  if (currentSetupState != SETUP_OFF) {
    checkSetupMenu();
    return; // Block UI/Timer updates while in setup menu
  }

  // Standalone: no longer wait/display connection state here

  unsigned long now = millis();

  if (exprTimeout > 0 && now >= exprTimeout) {
    exprTimeout = 0;
    setExpression(EXPR_IDLE);
  }

  if (tempMsgTimeout > 0 && now >= tempMsgTimeout) {
    tempMessage = "";
    tempMsgTimeout = 0;
    lastDrawnMsg = "";
    drawStatusLine();
  }

  if (currentExpr == EXPR_IDLE) {
    if (!isBlinking && (now - lastBlinkTime >= nextBlinkInterval)) {
      isBlinking = true;
      blinkStart = now;
      drawFace();
    }
    if (isBlinking && (now - blinkStart >= BLINK_DURATION)) {
      isBlinking = false;
      lastBlinkTime = now;
      nextBlinkInterval = random(2500, 5000);
      drawFace();
    }
  }

  if ((timerState == TIMER_FOCUS || timerState == TIMER_BREAK) &&
      timerSecondsRemaining > 0) {
    if (now - lastTimerTick >= 1000) {
      lastTimerTick = now;
      timerSecondsRemaining--;
      drawStatusLine();
      sendTimerState();

      if (timerSecondsRemaining == 0) {
        onTimerDone();
      }
    }
  }

  // Alarm Check (every 1 second approx)
  static unsigned long lastAlarmCheck = 0;
  if (now - lastAlarmCheck >= 1000) {
    lastAlarmCheck = now;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 0)) { // 0ms timeout (non-blocking if synced)
      if (currentDay != timeinfo.tm_mday) {
        currentDay = timeinfo.tm_mday;
        for (int i = 0; i < MAX_ALARMS; i++)
          alarms[i].triggeredToday = false;
      }

      for (int i = 0; i < MAX_ALARMS; i++) {
        if (alarms[i].enabled && !alarms[i].triggeredToday &&
            timeinfo.tm_hour == alarms[i].hour &&
            timeinfo.tm_min == alarms[i].minute) {
          triggerAlarm();
          alarms[i].triggeredToday = true;
        }
      }
    }
  }
}

void triggerAlarm() {
  setExpression(EXPR_HAPPY);
  tempMessage = "    ALARME!     ";
  tempMsgTimeout = millis() + 60000; // 60 seconds of message
  lastDrawnMsg = "";
  drawStatusLine();
  sendToClients("STATE:ALARM");
}

// ── Command Parser ─────────────────────────────────────────
void processCommand(String cmd) {
  cmd.trim();
  String rawCmd = cmd;
  cmd.toUpperCase();

  if (cmd.startsWith("EXP:")) {
    String expr = cmd.substring(4);
    if (expr == "IDLE")
      setExpression(EXPR_IDLE);
    else if (expr == "HAPPY")
      setExpression(EXPR_HAPPY);
    else if (expr == "SAD")
      setExpression(EXPR_SAD);
    else if (expr == "ANGRY")
      setExpression(EXPR_ANGRY);
    else if (expr == "FOCUS")
      setExpression(EXPR_FOCUS);
    else if (expr == "SLEEP")
      setExpression(EXPR_SLEEP);
    else if (expr == "LOVE")
      setExpression(EXPR_LOVE);
    else if (expr == "SURPRISED")
      setExpression(EXPR_SURPRISED);
    else if (expr == "STAR")
      setExpression(EXPR_STAR);
    else if (expr == "DIZZY")
      setExpression(EXPR_DIZZY);
    sendToClients("ACK:" + cmd);
  } else if (cmd.startsWith("CFG:POMO:")) {
    // Parse configs from Web UI: CFG:POMO:25:5:15
    String values = cmd.substring(9);
    int firstColon = values.indexOf(':');
    int secondColon = values.indexOf(':', firstColon + 1);

    if (firstColon != -1 && secondColon != -1) {
      focusDuration = values.substring(0, firstColon).toInt() * 60;
      shortBreakDuration =
          values.substring(firstColon + 1, secondColon).toInt() * 60;
      longBreakDuration = values.substring(secondColon + 1).toInt() * 60;

      preferences.putUInt("focus", focusDuration / 60);
      preferences.putUInt("sbreak", shortBreakDuration / 60);
      preferences.putUInt("lbreak", longBreakDuration / 60);

      Serial.println("Configuracoes sincronizadas pela Web!");

      // Echo back to all clients so other open tabs sync too
      String cfgMsg = "CFG:POMO:" + String(focusDuration / 60) + ":" +
                      String(shortBreakDuration / 60) + ":" +
                      String(longBreakDuration / 60);
      webSocket.broadcastTXT(cfgMsg);
    }
  } else if (cmd.startsWith("TMR:")) {
    String action = cmd.substring(4);
    if (action.startsWith("FOCUS")) {
      int duration = focusDuration;
      int colonIdx = action.indexOf(':');
      if (colonIdx != -1) {
        duration = action.substring(colonIdx + 1).toInt() * 60;
      }
      startTimer(TIMER_FOCUS, duration);
      setExpression(EXPR_FOCUS);
      exprTimeout = 0;
    } else if (action == "PAUSE") {
      if (timerState == TIMER_FOCUS)
        timerState = TIMER_FOCUS_PAUSED;
      else if (timerState == TIMER_BREAK)
        timerState = TIMER_BREAK_PAUSED;
      drawStatusLine();
    } else if (action == "RESUME") {
      if (timerState == TIMER_FOCUS_PAUSED) {
        timerState = TIMER_FOCUS;
        lastTimerTick = millis();
      } else if (timerState == TIMER_BREAK_PAUSED) {
        timerState = TIMER_BREAK;
        lastTimerTick = millis();
      }
      drawStatusLine();
    } else if (action == "STOP") {
      timerState = TIMER_OFF;
      timerSecondsRemaining = 0;
      setExpression(EXPR_IDLE);
      drawStatusLine();
    } else if (action.startsWith("BREAK")) {
      int duration = shortBreakDuration;
      int colonIdx = action.indexOf(':');
      if (colonIdx != -1) {
        duration = action.substring(colonIdx + 1).toInt() * 60;
      }
      startTimer(TIMER_BREAK, duration);
      setExpression(EXPR_HAPPY);
      exprTimeout = 0;
    } else if (action.startsWith("LBREAK")) {
      int duration = longBreakDuration;
      int colonIdx = action.indexOf(':');
      if (colonIdx != -1) {
        duration = action.substring(colonIdx + 1).toInt() * 60;
      }
      startTimer(TIMER_BREAK, duration);
      setExpression(EXPR_HAPPY);
      exprTimeout = 0;
    }
    sendToClients("ACK:" + cmd);
    sendTimerState();
  } else if (cmd.startsWith("MSG:")) {
    tempMessage = rawCmd.substring(4);
    if (tempMessage.length() > 16)
      tempMessage = tempMessage.substring(0, 16);
    tempMsgTimeout = millis() + 4000;
    lastDrawnMsg = "";
    drawStatusLine();
    sendToClients("ACK:MSG");
  } else if (cmd.startsWith("ALM:")) {
    String action = cmd.substring(4);
    int colonIdx = action.indexOf(':');
    String verb = (colonIdx != -1) ? action.substring(0, colonIdx) : action;

    if (verb == "SET") {
      int idIdx = action.indexOf(':', colonIdx + 1);
      if (idIdx != -1) {
        int id = action.substring(colonIdx + 1, idIdx).toInt();
        int nextColon = action.indexOf(':', idIdx + 1);
        if (id >= 0 && id < MAX_ALARMS && nextColon != -1) {
          alarms[id].hour = action.substring(idIdx + 1, nextColon).toInt();
          alarms[id].minute = action.substring(nextColon + 1).toInt();
          alarms[id].triggeredToday = false;
          saveAlarm(id);
          sendAlarmConfig(id);
        }
      }
    } else if (verb == "ON" || verb == "OFF" || verb == "DEL") {
      int id = action.substring(colonIdx + 1).toInt();
      if (id >= 0 && id < MAX_ALARMS) {
        if (verb == "ON") {
          alarms[id].enabled = true;
          alarms[id].triggeredToday = false;
        } else if (verb == "OFF") {
          alarms[id].enabled = false;
          if (tempMessage.indexOf("ALARME!") != -1) {
            tempMessage = "";
            tempMsgTimeout = 0;
            lastDrawnMsg = "";
            setExpression(EXPR_IDLE);
            drawStatusLine();
          }
        } else if (verb == "DEL") {
          alarms[id].enabled = false;
          alarms[id].hour = 0;
          alarms[id].minute = 0;
        }
        saveAlarm(id);
        sendAlarmConfig(id);
      }
    }
    sendToClients("ACK:" + cmd);
  }
}

// ── Expression Management ──────────────────────────────────
void setExpression(Expression expr) {
  currentExpr = expr;
  isBlinking = false;
  loadExpressionChars(expr);

  if (expr == EXPR_HAPPY || expr == EXPR_SAD || expr == EXPR_ANGRY) {
    if (timerState == TIMER_OFF || timerState == TIMER_DONE) {
      exprTimeout = millis() + 5000;
    }
  } else {
    exprTimeout = 0;
  }

  drawFace();

  String exprName;
  switch (expr) {
  case EXPR_IDLE:
    exprName = "IDLE";
    break;
  case EXPR_HAPPY:
    exprName = "HAPPY";
    break;
  case EXPR_SAD:
    exprName = "SAD";
    break;
  case EXPR_ANGRY:
    exprName = "ANGRY";
    break;
  case EXPR_FOCUS:
    exprName = "FOCUS";
    break;
  case EXPR_SLEEP:
    exprName = "SLEEP";
    break;
  case EXPR_LOVE:
    exprName = "LOVE";
    break;
  case EXPR_SURPRISED:
    exprName = "SURPRISED";
    break;
  case EXPR_STAR:
    exprName = "STAR";
    break;
  case EXPR_DIZZY:
    exprName = "DIZZY";
    break;
  }
  sendToClients("FACE:" + exprName);
}

void loadExpressionChars(Expression expr) {
  switch (expr) {
  case EXPR_IDLE:
    lcd.createChar(0, eyeLeftOpen);
    lcd.createChar(1, eyeRightOpen);
    lcd.createChar(2, eyeClosed);
    break;
  case EXPR_HAPPY:
    lcd.createChar(0, eyeHappyL);
    lcd.createChar(1, eyeHappyR);
    break;
  case EXPR_SAD:
    lcd.createChar(0, eyeSad);
    lcd.createChar(1, eyeSad);
    break;
  case EXPR_ANGRY:
    lcd.createChar(0, eyeAngry);
    lcd.createChar(1, eyeAngry);
    break;
  case EXPR_FOCUS:
    lcd.createChar(0, eyeFocus);
    lcd.createChar(1, eyeFocus);
    break;
  case EXPR_SLEEP:
    lcd.createChar(0, eyeClosed);
    lcd.createChar(1, eyeClosed);
    break;
  case EXPR_LOVE:
    lcd.createChar(0, eyeLove);
    lcd.createChar(1, eyeLove);
    break;
  case EXPR_SURPRISED:
    lcd.createChar(0, eyeSurprised);
    lcd.createChar(1, eyeSurprised);
    break;
  case EXPR_STAR:
    lcd.createChar(0, eyeStar);
    lcd.createChar(1, eyeStar);
    break;
  case EXPR_DIZZY:
    lcd.createChar(0, eyeDizzy);
    lcd.createChar(1, eyeDizzy);
    break;
  }
}

// ── Draw Functions ─────────────────────────────────────────
void drawFace() {

  lcd.setCursor(0, 0);
  lcd.print("                ");

  if (currentExpr == EXPR_SLEEP) {
    lcd.setCursor(4, 0);
    lcd.print("-");
    lcd.setCursor(6, 0);
    lcd.print("_");
    lcd.setCursor(8, 0);
    lcd.print("-");
    lcd.setCursor(11, 0);
    lcd.print("z Z");
  } else if (currentExpr == EXPR_IDLE && isBlinking) {
    lcd.setCursor(5, 0);
    lcd.write((byte)2);
    lcd.setCursor(10, 0);
    lcd.write((byte)2);
  } else {
    lcd.setCursor(5, 0);
    lcd.write((byte)0);
    lcd.setCursor(10, 0);
    lcd.write((byte)1);

    if (currentExpr == EXPR_HAPPY) {
      lcd.setCursor(7, 0);
      lcd.print("v");
      lcd.setCursor(8, 0);
      lcd.print("v");
    } else if (currentExpr == EXPR_ANGRY) {
      lcd.setCursor(7, 0);
      lcd.print(">");
      lcd.setCursor(8, 0);
      lcd.print("<");
    }
  }

  lastDrawnExpr = currentExpr;
  lastDrawnBlink = isBlinking;
}

void drawStatusLine() {

  lcd.setCursor(0, 1);

  if (tempMessage.length() > 0) {
    String padded = tempMessage;
    while (padded.length() < 16)
      padded += ' ';
    if (padded != lastDrawnMsg) {
      lcd.print(padded);
      lastDrawnMsg = padded;
    }
    return;
  }

  String line = "";

  switch (timerState) {
  case TIMER_OFF:
    line = "                ";
    break;
  case TIMER_FOCUS: {
    unsigned int m = timerSecondsRemaining / 60;
    unsigned int s = timerSecondsRemaining % 60;
    line = "  FOCO  ";
    if (m < 10)
      line += "0";
    line += String(m) + ":";
    if (s < 10)
      line += "0";
    line += String(s) + "   ";
    break;
  }
  case TIMER_FOCUS_PAUSED: {
    unsigned int m = timerSecondsRemaining / 60;
    unsigned int s = timerSecondsRemaining % 60;
    line = " PAUSADO ";
    if (m < 10)
      line += "0";
    line += String(m) + ":";
    if (s < 10)
      line += "0";
    line += String(s) + "  ";
    break;
  }
  case TIMER_BREAK: {
    unsigned int m = timerSecondsRemaining / 60;
    unsigned int s = timerSecondsRemaining % 60;
    line = " PAUSA  ";
    if (m < 10)
      line += "0";
    line += String(m) + ":";
    if (s < 10)
      line += "0";
    line += String(s) + "   ";
    break;
  }
  case TIMER_BREAK_PAUSED: {
    unsigned int m = timerSecondsRemaining / 60;
    unsigned int s = timerSecondsRemaining % 60;
    line = " PAUSADO ";
    if (m < 10)
      line += "0";
    line += String(m) + ":";
    if (s < 10)
      line += "0";
    line += String(s) + "  ";
    break;
  }
  case TIMER_DONE:
    line = "   DONE! :D     ";
    break;
  }

  while (line.length() < 16)
    line += ' ';
  line = line.substring(0, 16);

  if (line != lastDrawnMsg) {
    lcd.print(line);
    lastDrawnMsg = line;
  }
}

// ── Timer Functions ────────────────────────────────────────
void startTimer(TimerState state, unsigned long seconds) {
  timerState = state;
  timerSecondsRemaining = seconds;
  lastTimerTick = millis();
  drawStatusLine();
}

void onTimerDone() {
  if (timerState == TIMER_FOCUS) {
    timerState = TIMER_DONE;
    setExpression(EXPR_HAPPY);
    exprTimeout = 0;
    sendToClients("STATE:DONE");

    tempMessage = "FOCO CONCLUIDO! ";
    tempMsgTimeout = millis() + 5000;
    lastDrawnMsg = "";
    drawStatusLine();
  } else if (timerState == TIMER_BREAK) {
    timerState = TIMER_DONE;
    setExpression(EXPR_IDLE);
    sendToClients("STATE:DONE");

    tempMessage = " PAUSA ACABOU!  ";
    tempMsgTimeout = millis() + 4000;
    lastDrawnMsg = "";
    drawStatusLine();
  }
}

void sendTimerState() {
  String msg = "STATE:";
  unsigned int m = timerSecondsRemaining / 60;
  unsigned int s = timerSecondsRemaining % 60;

  switch (timerState) {
  case TIMER_OFF:
    msg += "IDLE";
    break;
  case TIMER_FOCUS:
  case TIMER_FOCUS_PAUSED: {
    msg += "FOCUS:";
    if (m < 10)
      msg += "0";
    msg += String(m) + ":";
    if (s < 10)
      msg += "0";
    msg += String(s);
    if (timerState == TIMER_FOCUS_PAUSED)
      msg += ":PAUSED";
    break;
  }
  case TIMER_BREAK:
  case TIMER_BREAK_PAUSED: {
    msg += "BREAK:";
    if (m < 10)
      msg += "0";
    msg += String(m) + ":";
    if (s < 10)
      msg += "0";
    msg += String(s);
    if (timerState == TIMER_BREAK_PAUSED)
      msg += ":PAUSED";
    break;
  }
  case TIMER_DONE:
    msg += "DONE";
    break;
  }
  sendToClients(msg);
}
