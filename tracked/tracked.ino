// ============================================================
// Tracked Remote — ESP32 + LCD 1602A (MagicTracked Remote Controller via MQTT)
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
//   BTN_PREV  (GPIO 26) → Item Anterior
//   BTN_NEXT  (GPIO 25) → Próximo Item
//   BTN_MENU  (GPIO 27) → Alterna entre 4 Modos (Segure ao ligar para Resetar
//   Wi-Fi/Portal) BTN_APPLY (GPIO 32) → Aplicar Efeito/Preset ou Disparar Ação
//   POT_PIN   (GPIO 34) → Ajuste em Tempo Real dos Parâmetros
// ============================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <LiquidCrystal.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <ESPmDNS.h>

Preferences preferences;

// ── Pin Configuration ──────────────────────────────────────
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);
#define BTN_PREV_PIN 26
#define BTN_NEXT_PIN 25
#define BTN_MENU_PIN 27
#define BTN_APPLY_PIN 32
#define POT_PIN 34

// ── Networking & MQTT (TLS / SSL para EMQX Cloud) ─────────
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

char brokerIp[64] = "a1d0120f.ala.us-east-1.emqxsl.com";
const int MQTT_PORT = 8883; // Porta TLS/SSL do EMQX Cloud
bool shouldSaveConfig = false;

// Callback para salvar configurações do WiFiManager
void saveParamsCallback() { shouldSaveConfig = true; }

// ── Busca de Hostname por mDNS ─────────────────────────────
bool resolveMacAddress() {
  Serial.println("[mDNS] Buscando 'magictracked.local' na rede Wi-Fi...");
  IPAddress resolvedIp;

  // Tenta resolver via mDNS nativo com timeout curto (300ms) para não travar o
  // loop
  resolvedIp = MDNS.queryHost("magictracked.local", 300);

  if (resolvedIp == INADDR_NONE || resolvedIp == IPAddress(0, 0, 0, 0)) {
    WiFi.hostByName("magictracked.local", resolvedIp);
  }

  if (resolvedIp != INADDR_NONE && resolvedIp != IPAddress(0, 0, 0, 0)) {
    snprintf(brokerIp, sizeof(brokerIp), "%d.%d.%d.%d", resolvedIp[0],
             resolvedIp[1], resolvedIp[2], resolvedIp[3]);
    Serial.printf("[mDNS] Mac encontrado em: %s\n", brokerIp);
    return true;
  }

  Serial.println("[mDNS] Não foi possível resolver magictracked.local. Usando "
                 "IP salvo/fallback.");
  return false;
}

// ── Remote Control Modes ───────────────────────────────────
enum ControllerMode {
  MODE_EFFECTS = 0,
  MODE_PRESETS = 1,
  MODE_PARAMS = 2,
  MODE_ACTIONS = 3
};

ControllerMode currentMode = MODE_EFFECTS;
int selectedIndex = 0;

// ── Data Lists ─────────────────────────────────────────────
struct EffectItem {
  const char *displayName;
  const char *cmdId;
};

EffectItem EFFECTS_LIST[] = {{"Motion Veil", "motionVeil"},
                             {"Liquid Ripple", "liquidRipple"},
                             {"Thermal Vision", "thermalVision"},
                             {"TouchDesigner", "touchDesignerPortal"},
                             {"ASCII Art", "ascii"},
                             {"Glitch Effect", "glitch"},
                             {"Effect Circle", "circle"}};
const int TOTAL_EFFECTS = 7;

struct PresetItem {
  const char *displayName;
  const char *cmdId;
};

PresetItem PRESETS_LIST[] = {{"Brik Original", "brik"},
                             {"Liquid Glass", "liquidGlass"},
                             {"Neon Prism", "neonPrism"},
                             {"Rainbow Silk", "silkCloth"}};
const int TOTAL_PRESETS = 4;

struct ParamItem {
  const char *displayName;
  const char *paramId;
  float minVal;
  float maxVal;
  bool isInt;
};

ParamItem PARAMS_LIST[] = {
    {"Iridescencia", "uIridescence", 0.0f, 1.0f, false},
    {"Dispersao RGB", "uChromaticDispersion", 0.0f, 0.15f, false},
    {"Ondulacao", "uWaveRipple", 0.0f, 1.0f, false},
    {"Opacidade", "uOpacity", 0.0f, 1.0f, false},
    {"Refracao", "uRefractionStrength", 0.0f, 150.0f, true},
    {"Esqueleto", "skeletonLineWidth", 1.0f, 6.0f, false}};
const int TOTAL_PARAMS = 6;

struct ActionItem {
  const char *displayName;
  const char *cmdId;
};

ActionItem ACTIONS_LIST[] = {{"Play / Pause", "play"},
                             {"Gravar Tela", "rec_toggle"},
                             {"Reset Controls", "reset"},
                             {"Tela Cheia", "fullscreen"}};
const int TOTAL_ACTIONS = 4;

// ── Potentiometer State ────────────────────────────────────
int lastPotRaw = -1;
unsigned long lastPotSendTime = 0;

// ── Button Debounce ────────────────────────────────────────
unsigned long lastBtnMenuTime = 0;
unsigned long lastBtnPrevTime = 0;
unsigned long lastBtnNextTime = 0;
unsigned long lastBtnApplyTime = 0;
const unsigned long BTN_COOLDOWN = 250;

// ── LCD Status Overlay ─────────────────────────────────────
String overlayMsg = "";
unsigned long overlayTimeout = 0;

// ── MQTT Callback for incoming status from Mac ──────────────
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.printf("[MQTT RX] %s: %s\n", topic, msg.c_str());

  if (String(topic) == "magictracked/status/app") {
    overlayMsg = "Mac: Conectado";
    overlayTimeout = millis() + 2000;
  }
}

void reconnectMQTT() {
  if (!mqttClient.connected()) {
    mqttClient.setServer(brokerIp, MQTT_PORT);

    Serial.print("Conectando ao Broker MQTT (");
    Serial.print(brokerIp);
    Serial.println(")...");

    String clientId = "ESP32_TrackedRemote_" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), "tracked_user",
                           "tufjow-racxof-roRxo9")) {
      Serial.println("MQTT Conectado com sucesso!");
      mqttClient.subscribe("magictracked/status/#");
      mqttClient.publish("magictracked/cmd/action", "connect_ack");
    } else {
      Serial.print("Falha na conexao MQTT, rc=");
      Serial.println(mqttClient.state());
    }
  }
}

// ── LCD UI Rendering ───────────────────────────────────────
void updateLcdDisplay() {
  if (overlayTimeout > 0 && millis() < overlayTimeout) {
    lcd.setCursor(0, 0);
    lcd.print("     STATUS     ");
    lcd.setCursor(0, 1);
    String padded = overlayMsg;
    while (padded.length() < 16)
      padded += ' ';
    lcd.print(padded.substring(0, 16));
    return;
  }

  char line1[17];
  char line2[17];

  switch (currentMode) {
  case MODE_EFFECTS: {
    snprintf(line1, sizeof(line1), "[EFEITO] %d/%d   ", selectedIndex + 1,
             TOTAL_EFFECTS);
    snprintf(line2, sizeof(line2), "%-16s",
             EFFECTS_LIST[selectedIndex].displayName);
    break;
  }
  case MODE_PRESETS: {
    snprintf(line1, sizeof(line1), "[PRESET] %d/%d   ", selectedIndex + 1,
             TOTAL_PRESETS);
    snprintf(line2, sizeof(line2), "%-16s",
             PRESETS_LIST[selectedIndex].displayName);
    break;
  }
  case MODE_PARAMS: {
    ParamItem item = PARAMS_LIST[selectedIndex];
    int potVal = analogRead(POT_PIN);
    float norm = (float)potVal / 4095.0f;
    float calculatedVal = item.minVal + norm * (item.maxVal - item.minVal);

    snprintf(line1, sizeof(line1), "[PAR] %-10s", item.displayName);
    if (item.isInt) {
      snprintf(line2, sizeof(line2), "Val: %-5d      ", (int)calculatedVal);
    } else {
      snprintf(line2, sizeof(line2), "Val: %-5.2f     ", calculatedVal);
    }
    break;
  }
  case MODE_ACTIONS: {
    snprintf(line1, sizeof(line1), "[AÇÃO] %d/%d     ", selectedIndex + 1,
             TOTAL_ACTIONS);
    snprintf(line2, sizeof(line2), "%-16s",
             ACTIONS_LIST[selectedIndex].displayName);
    break;
  }
  }

  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ── Publish MQTT Command ───────────────────────────────────
void sendMqttCommand(const char *subtopic, const char *payload) {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }

  if (mqttClient.connected()) {
    String topic = "magictracked/cmd/" + String(subtopic);
    mqttClient.publish(topic.c_str(), payload);
    Serial.printf("[MQTT TX] %s -> %s\n", topic.c_str(), payload);

    overlayMsg = "-> ENVIADO!     ";
    overlayTimeout = millis() + 1000;
    updateLcdDisplay();
  }
}

// ── Handle Buttons ─────────────────────────────────────────
void checkButtons() {
  unsigned long now = millis();

  // 1. BTN MENU (GPIO 27) -> Switch Mode
  if (digitalRead(BTN_MENU_PIN) == LOW) {
    if (now - lastBtnMenuTime > BTN_COOLDOWN) {
      lastBtnMenuTime = now;
      currentMode = (ControllerMode)((currentMode + 1) % 4);
      selectedIndex = 0;
      updateLcdDisplay();
    }
  }

  // 2. BTN PREV (GPIO 26) -> Previous Item
  if (digitalRead(BTN_PREV_PIN) == LOW) {
    if (now - lastBtnPrevTime > BTN_COOLDOWN) {
      lastBtnPrevTime = now;
      int maxItems = (currentMode == MODE_EFFECTS)   ? TOTAL_EFFECTS
                     : (currentMode == MODE_PRESETS) ? TOTAL_PRESETS
                     : (currentMode == MODE_PARAMS)  ? TOTAL_PARAMS
                                                     : TOTAL_ACTIONS;

      selectedIndex = (selectedIndex - 1 + maxItems) % maxItems;
      updateLcdDisplay();
    }
  }

  // 3. BTN NEXT (GPIO 25) -> Next Item
  if (digitalRead(BTN_NEXT_PIN) == LOW) {
    if (now - lastBtnNextTime > BTN_COOLDOWN) {
      lastBtnNextTime = now;
      int maxItems = (currentMode == MODE_EFFECTS)   ? TOTAL_EFFECTS
                     : (currentMode == MODE_PRESETS) ? TOTAL_PRESETS
                     : (currentMode == MODE_PARAMS)  ? TOTAL_PARAMS
                                                     : TOTAL_ACTIONS;

      selectedIndex = (selectedIndex + 1) % maxItems;
      updateLcdDisplay();
    }
  }

  // 4. BTN APPLY (GPIO 32) -> Confirm / Trigger Action
  if (digitalRead(BTN_APPLY_PIN) == LOW) {
    if (now - lastBtnApplyTime > BTN_COOLDOWN) {
      lastBtnApplyTime = now;

      if (currentMode == MODE_EFFECTS) {
        sendMqttCommand("effect", EFFECTS_LIST[selectedIndex].cmdId);
      } else if (currentMode == MODE_PRESETS) {
        sendMqttCommand("preset", PRESETS_LIST[selectedIndex].cmdId);
      } else if (currentMode == MODE_ACTIONS) {
        sendMqttCommand("action", ACTIONS_LIST[selectedIndex].cmdId);
      }
    }
  }
}

// ── Handle Potentiometer Smooth Reading ────────────────────
void checkPotentiometer() {
  if (currentMode != MODE_PARAMS)
    return;

  unsigned long now = millis();
  int raw = analogRead(POT_PIN);

  if (abs(raw - lastPotRaw) > 30) {
    lastPotRaw = raw;
    updateLcdDisplay();

    if (now - lastPotSendTime > 80) {
      lastPotSendTime = now;

      ParamItem item = PARAMS_LIST[selectedIndex];
      float norm = (float)raw / 4095.0f;
      float calculatedVal = item.minVal + norm * (item.maxVal - item.minVal);

      char jsonPayload[64];
      if (item.isInt) {
        snprintf(jsonPayload, sizeof(jsonPayload),
                 "{\"param\":\"%s\",\"value\":%d}", item.paramId,
                 (int)calculatedVal);
      } else {
        snprintf(jsonPayload, sizeof(jsonPayload),
                 "{\"param\":\"%s\",\"value\":%.3f}", item.paramId,
                 calculatedVal);
      }

      sendMqttCommand("param", jsonPayload);
    }
  }
}

// ── Setup ──────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  preferences.begin("tracked_remote", false);
  String savedIp = preferences.getString("broker_ip", "a1d0120f.ala.us-east-1.emqxsl.com");
  
  // Garante que o IP seja o EMQX Cloud (se houver qualquer IP numérico antigo salvo na Flash)
  if (!savedIp.endsWith("emqxsl.com")) {
    savedIp = "a1d0120f.ala.us-east-1.emqxsl.com";
    preferences.putString("broker_ip", savedIp);
  }
  savedIp.toCharArray(brokerIp, sizeof(brokerIp));

  pinMode(BTN_PREV_PIN, INPUT_PULLUP);
  pinMode(BTN_NEXT_PIN, INPUT_PULLUP);
  pinMode(BTN_MENU_PIN, INPUT_PULLUP);
  pinMode(BTN_APPLY_PIN, INPUT_PULLUP);

  pinMode(13, OUTPUT);
  analogWrite(13, 75);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" MagicTracked ");
  lcd.setCursor(0, 1);
  lcd.print("Remote Controller");
  delay(1200);

  // WiFiManager com Campo Personalizado para o Broker MQTT
  WiFiManager wifiManager;
  wifiManager.setSaveParamsCallback(saveParamsCallback);
  wifiManager.setConnectTimeout(15);

  WiFiManagerParameter custom_broker_ip("broker_ip", "Host MQTT Broker",
                                        brokerIp, 64);
  wifiManager.addParameter(&custom_broker_ip);

  // Se o Botão MENU (GPIO 27) estiver Pressionado ao Ligar, Força o Reset do
  // Wi-Fi e abre o Portal Captivo!
  if (digitalRead(BTN_MENU_PIN) == LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Resetando Wi-Fi");
    lcd.setCursor(0, 1);
    lcd.print(" Abrindo Portal ");
    delay(2000);
    wifiManager.resetSettings();
  }

  if (!wifiManager.autoConnect("MagicTracked-Remote")) {
    Serial.println("Falha ao conectar no Wi-Fi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  Wi-Fi Erro!   ");
    delay(2000);
    ESP.restart();
  }

  // Se o usuário preencheu/alterou o IP do Mac no portal captivo
  if (shouldSaveConfig) {
    String newIp = String(custom_broker_ip.getValue());
    newIp.trim();
    if (newIp.length() > 0) {
      newIp.toCharArray(brokerIp, sizeof(brokerIp));
      preferences.putString("broker_ip", brokerIp);
      Serial.println("Novo Host MQTT salvo na Flash: " + newIp);
    }
  }

  Serial.println("\nWi-Fi Conectado!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  // Iniciar cliente mDNS no ESP32 antes de fazer consultas
  if (MDNS.begin("trackedremote")) {
    Serial.println("Responder mDNS do ESP32 iniciado (trackedremote.local)");
  }

  // 🔍 Se for o broker em nuvem EMQX, conecta direto sem tentar mDNS local
  if (String(brokerIp).indexOf("emqxsl.com") >= 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  EMQX Cloud   ");
    lcd.setCursor(0, 1);
    lcd.print(" Conectando...  ");
    delay(1000);
  } else if (resolveMacAddress()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Mac Encontrado!");
    lcd.setCursor(0, 1);
    lcd.print(brokerIp);
    delay(1500);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Usando IP Salvo");
    lcd.setCursor(0, 1);
    lcd.print(brokerIp);
    delay(1500);
  }

  // Configura TLS sem validação de certificado CA (ideal para ESP32 conectar na porta 8883 do EMQX)
  espClient.setInsecure();

  mqttClient.setServer(brokerIp, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  // Conecta imediatamente ao MQTT no boot
  reconnectMQTT();

  lcd.clear();
  updateLcdDisplay();
}

// ── Main Loop ──────────────────────────────────────────────
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      static unsigned long lastMqttRetry = 0;
      if (millis() - lastMqttRetry > 5000) {
        lastMqttRetry = millis();
        reconnectMQTT();
      }
    } else {
      mqttClient.loop();
    }
  }

  checkButtons();
  checkPotentiometer();

  if (overlayTimeout > 0 && millis() >= overlayTimeout) {
    overlayTimeout = 0;
    updateLcdDisplay();
  }

  delay(10);
}
