// ============================================================
// Trenzin — ESP32 + LCD 1602A (Wi-Fi WebSocket)
// A cute desk buddy with expressions & Pomodoro timer
// ============================================================
// Wiring (4-bit parallel mode for ESP32 - 3.3V Logic):
//   LCD RS  → GPIO 19   LCD Enable → GPIO 23
//   LCD D4  → GPIO 18   LCD D5     → GPIO 17
//   LCD D6  → GPIO 16   LCD D7     → GPIO 15
//   LCD R/W → GND       (CRITICAL! Must be GND to protect ESP32)
//   LCD VDD → 5V (VIN)  LCD VSS    → GND
//   Contrast pot (10kΩ) on V0 or GND directly
// ============================================================

#include "web_assets.h"
#include <ESPmDNS.h>
#include <LiquidCrystal.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <WiFiManager.h>

// ── Pin Configuration ──────────────────────────────────────
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);

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
  EXPR_SLEEP
};

enum TimerState {
  TIMER_OFF,
  TIMER_FOCUS,
  TIMER_FOCUS_PAUSED,
  TIMER_BREAK,
  TIMER_BREAK_PAUSED,
  TIMER_DONE
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

// Timer durations
const unsigned long FOCUS_DURATION = 25UL * 60; // 25 min in seconds
const unsigned long BREAK_DURATION = 5UL * 60;  // 5 min in seconds

// ── Temporary Message ──────────────────────────────────────
String tempMessage = "";
unsigned long tempMsgTimeout = 0;

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

// ── Setup ──────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Truque mágico do Contraste (PWM) no Pino 13
  pinMode(13, OUTPUT);
  // O valor vai de 0 (máximo escuro/GND) a 255 (apagado).
  // 60 costuma ser o ponto doce perfeito! Mude se precisar.
  analogWrite(13, 75);

  lcd.begin(16, 2);

  loadExpressionChars(EXPR_IDLE);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Trenzin OS 2.0");
  lcd.setCursor(0, 1);
  lcd.print("Conectando...");

  // Connect to Wi-Fi using WiFiManager
  WiFiManager wifiManager;
  // Opcional: Se precisar resetar as configurações de Wi-Fi salvas para testar
  // o portal cativo, descomente a linha abaixo.
  wifiManager.resetSettings();

  // Tenta conectar nas redes conhecidas.
  // Se falhar ou não houver redes salvas, ele sobe um Access Point chamado
  // "Trenzin-Setup"
  if (!wifiManager.autoConnect("trenzin-by-magicoven")) {
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

  // Setup Web Server Routes
  server.on("/", []() { server.send(200, "text/html", WEB_HTML); });
  server.on("/style.css", []() { server.send(200, "text/css", WEB_CSS); });
  server.on("/app.js",
            []() { server.send(200, "application/javascript", WEB_JS); });
  server.begin();

  // Initialize UI
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aguardando app..");
  lcd.setCursor(0, 1);
  lcd.print("trenzin.local");

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
    break;
  case WStype_CONNECTED: {
    connectedClients++;
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2],
                  ip[3]);
    // Send initial state to the new client
    sendToClients("ACK:BOOT");
    sendToClients("STATE:IDLE");
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

  bool currentConnectionState = (connectedClients > 0);
  if (currentConnectionState != lastConnectionState) {
    lastConnectionState = currentConnectionState;
    lcd.clear();
    if (!currentConnectionState) {
      lcd.setCursor(0, 0);
      lcd.print("Aguardando app..");
      lcd.setCursor(0, 1);
      lcd.print("trenzin.local");
    } else {
      lastDrawnExpr = (Expression)255;
      lastDrawnMsg = "";
      drawFace();
      drawStatusLine();
    }
  }

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
    sendToClients("ACK:" + cmd);
  } else if (cmd.startsWith("TMR:")) {
    String action = cmd.substring(4);
    if (action == "START") {
      startTimer(TIMER_FOCUS, FOCUS_DURATION);
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
    } else if (action == "BREAK") {
      startTimer(TIMER_BREAK, BREAK_DURATION);
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
  }
}

// ── Draw Functions ─────────────────────────────────────────
void drawFace() {
  if (connectedClients == 0)
    return;

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
  if (connectedClients == 0)
    return;

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
    line = " FOCUS  ";
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
    line = " PAUSED ";
    if (m < 10)
      line += "0";
    line += String(m) + ":";
    if (s < 10)
      line += "0";
    line += String(s) + "   ";
    break;
  }
  case TIMER_BREAK: {
    unsigned int m = timerSecondsRemaining / 60;
    unsigned int s = timerSecondsRemaining % 60;
    line = " BREAK  ";
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
    line = " PAUSE  ";
    if (m < 10)
      line += "0";
    line += String(m) + ":";
    if (s < 10)
      line += "0";
    line += String(s) + "   ";
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

    tempMessage = "  FOCUS DONE!   ";
    tempMsgTimeout = millis() + 5000;
    lastDrawnMsg = "";
    drawStatusLine();
  } else if (timerState == TIMER_BREAK) {
    timerState = TIMER_DONE;
    setExpression(EXPR_IDLE);
    sendToClients("STATE:DONE");

    tempMessage = "  BREAK OVER!   ";
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
