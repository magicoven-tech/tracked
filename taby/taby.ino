// ============================================================
// TABY Clone — Arduino Nano + LCD 1602A
// A cute desk buddy with expressions & Pomodoro timer
// ============================================================
// Wiring (4-bit parallel mode):
//   LCD RS  → Pin 12    LCD Enable → Pin 11
//   LCD D4  → Pin 5     LCD D5     → Pin 4
//   LCD D6  → Pin 3     LCD D7     → Pin 2
//   LCD R/W → GND
//   Contrast pot (10kΩ) on V0
// ============================================================

#include <LiquidCrystal.h>

// ── Pin Configuration ──────────────────────────────────────
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

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

// ── Serial Buffer ──────────────────────────────────────────
String serialBuffer = "";

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
// Track what's drawn so we only update when changed
Expression lastDrawnExpr = (Expression)255;
bool lastDrawnBlink = false;
TimerState lastDrawnTimerState = (TimerState)255;
unsigned long lastDrawnSeconds = 999999;
String lastDrawnMsg = "";

// ── Setup ──────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  // Register default custom characters
  loadExpressionChars(EXPR_IDLE);

  lcd.clear();
  drawFace();
  drawStatusLine();

  Serial.println("ACK:BOOT");
  Serial.println("STATE:IDLE");

  lastBlinkTime = millis();
  nextBlinkInterval = random(2500, 5000);
}

// ── Main Loop ──────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // ── Read Serial ──
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialBuffer.length() > 0) {
        processCommand(serialBuffer);
        serialBuffer = "";
      }
    } else {
      serialBuffer += c;
    }
  }

  // ── Expression auto-return to idle ──
  if (exprTimeout > 0 && now >= exprTimeout) {
    exprTimeout = 0;
    setExpression(EXPR_IDLE);
  }

  // ── Temp message timeout ──
  if (tempMsgTimeout > 0 && now >= tempMsgTimeout) {
    tempMessage = "";
    tempMsgTimeout = 0;
    lastDrawnMsg = ""; // force redraw
    drawStatusLine();
  }

  // ── Blink animation (only in IDLE) ──
  if (currentExpr == EXPR_IDLE) {
    if (!isBlinking && (now - lastBlinkTime >= nextBlinkInterval)) {
      isBlinking = true;
      blinkStart = now;
      drawFace(); // draw closed eyes
    }
    if (isBlinking && (now - blinkStart >= BLINK_DURATION)) {
      isBlinking = false;
      lastBlinkTime = now;
      nextBlinkInterval = random(2500, 5000);
      drawFace(); // draw open eyes
    }
  }

  // ── Timer countdown ──
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

  // Save raw command before uppercasing (preserves MSG text case)
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
    Serial.println("ACK:" + cmd);
  } else if (cmd.startsWith("TMR:")) {
    String action = cmd.substring(4);
    if (action == "START") {
      startTimer(TIMER_FOCUS, FOCUS_DURATION);
      setExpression(EXPR_FOCUS);
      exprTimeout = 0; // don't auto-return during focus
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
    Serial.println("ACK:" + cmd);
    sendTimerState();
  } else if (cmd.startsWith("MSG:")) {
    // Use rawCmd to preserve original case for the message text
    tempMessage = rawCmd.substring(4);
    if (tempMessage.length() > 16)
      tempMessage = tempMessage.substring(0, 16);
    tempMsgTimeout = millis() + 4000; // show for 4 seconds
    lastDrawnMsg = "";
    drawStatusLine();
    Serial.println("ACK:MSG");
  }
}

// ── Expression Management ──────────────────────────────────
void setExpression(Expression expr) {
  currentExpr = expr;
  isBlinking = false;
  loadExpressionChars(expr);

  // Auto-return timeout for transient expressions
  if (expr == EXPR_HAPPY || expr == EXPR_SAD || expr == EXPR_ANGRY) {
    // Only auto-return if timer is not running
    if (timerState == TIMER_OFF || timerState == TIMER_DONE) {
      exprTimeout = millis() + 5000;
    }
  } else {
    exprTimeout = 0;
  }

  drawFace();

  // Send state
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
  Serial.println("FACE:" + exprName);
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
  // Row 0: face expression
  // Layout: "  L    R   status"
  // Positions: eye_L at col 5, eye_R at col 10

  lcd.setCursor(0, 0);
  lcd.print("                "); // clear row 0

  if (currentExpr == EXPR_SLEEP) {
    // Sleeping face: -_- z Z
    lcd.setCursor(4, 0);
    lcd.print("-");
    lcd.setCursor(6, 0);
    lcd.print("_");
    lcd.setCursor(8, 0);
    lcd.print("-");
    lcd.setCursor(11, 0);
    lcd.print("z Z");
  } else if (currentExpr == EXPR_IDLE && isBlinking) {
    // Blinking: show closed eyes
    lcd.setCursor(5, 0);
    lcd.write((byte)2); // closed
    lcd.setCursor(10, 0);
    lcd.write((byte)2); // closed
  } else {
    // Normal: show expression eyes
    lcd.setCursor(5, 0);
    lcd.write((byte)0); // left eye
    lcd.setCursor(10, 0);
    lcd.write((byte)1); // right eye

    // Add mouth/extras for some expressions
    if (currentExpr == EXPR_HAPPY) {
      lcd.setCursor(7, 0);
      lcd.print("v");
      lcd.setCursor(8, 0);
      lcd.print("v");
    } else if (currentExpr == EXPR_SAD) {
      // sad mouth on row 0 isn't great, skip
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

  // If there's a temporary message, show it
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

  // Pad to 16 chars
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
    // Focus done — celebrate!
    timerState = TIMER_DONE;
    setExpression(EXPR_HAPPY);
    exprTimeout = 0;
    Serial.println("STATE:DONE");

    // Flash "DONE!" message
    tempMessage = "  FOCUS DONE!   ";
    tempMsgTimeout = millis() + 5000;
    lastDrawnMsg = "";
    drawStatusLine();
  } else if (timerState == TIMER_BREAK) {
    // Break done — ready to go again
    timerState = TIMER_DONE;
    setExpression(EXPR_IDLE);
    Serial.println("STATE:DONE");

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
  Serial.println(msg);
}
