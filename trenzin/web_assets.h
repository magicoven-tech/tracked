// Arquivo gerado automaticamente
// NÃO EDITE MANUALMENTE!

#ifndef WEB_ASSETS_H
#define WEB_ASSETS_H

#include <pgmspace.h>

const char WEB_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="pt-BR">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0">
  <title>Trenzin - Seu companheiro de mesa</title>
  <meta name="description"
    content="Control your Trenzin desk robot. Manage expressions, Pomodoro timer, and send messages via USB Serial.">

  <!-- Fonts -->
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&display=swap" rel="stylesheet">

  <!-- Styles -->
  <link rel="stylesheet" href="style.css">

  <!-- Favicon -->
  <link rel="icon"
    href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='80' font-size='80'>🤖</text></svg>">
</head>

<body>
  <div class="app">

    <!-- ── Header ──────────────────────────────────────── -->
    <header class="header animate-in" id="header">
      <div class="header__brand">
        <h1 class="header__logo">TREN<span>ZIN</span></h1>
        <span class="header__tagline">Seu companheiro de mesa</span>
      </div>
      <div class="connection-group">

        <button class="connect-btn" id="connect-btn">
          <span class="connect-btn__dot"></span>
          <span class="connect-btn__text">Connect Trenzin</span>
        </button>
      </div>
    </header>

    <!-- ── Status Bar ──────────────────────────────────── -->
    <div class="status-bar animate-in" id="status-bar">
      <div class="status-chip" id="status-connection">
        <span class="status-chip__icon">⚡</span>
        <span class="status-chip__text">Offline</span>
      </div>
      <div class="status-chip" id="status-expression">
        <span class="status-chip__icon">😊</span>
        <span class="status-chip__text">Idle</span>
      </div>
    </div>

    <!-- ── Main Grid ───────────────────────────────────── -->
    <div class="grid">

      <!-- Expressions Card -->
      <div class="card animate-in" id="card-expressions">
        <span class="card__label">Expressions</span>
        <div class="expressions-grid">
          <button class="expr-btn expr-btn--active" data-expr="idle" id="expr-idle">
            <span class="expr-btn__emoji">😊</span>
            <span class="expr-btn__label">Idle</span>
          </button>
          <button class="expr-btn" data-expr="happy" id="expr-happy">
            <span class="expr-btn__emoji">😄</span>
            <span class="expr-btn__label">Happy</span>
          </button>
          <button class="expr-btn" data-expr="sad" id="expr-sad">
            <span class="expr-btn__emoji">😢</span>
            <span class="expr-btn__label">Sad</span>
          </button>
          <button class="expr-btn" data-expr="angry" id="expr-angry">
            <span class="expr-btn__emoji">😠</span>
            <span class="expr-btn__label">Angry</span>
          </button>
          <button class="expr-btn" data-expr="focus" id="expr-focus">
            <span class="expr-btn__emoji">🎯</span>
            <span class="expr-btn__label">Focus</span>
          </button>
          <button class="expr-btn" data-expr="sleep" id="expr-sleep">
            <span class="expr-btn__emoji">😴</span>
            <span class="expr-btn__label">Sleep</span>
          </button>
        </div>
      </div>

      <!-- LCD Preview Card -->
      <div class="card animate-in" id="card-lcd">
        <span class="card__label">LCD Preview</span>
        <div class="lcd-preview">
          <div class="lcd">
            <div class="lcd__row" id="lcd-row-0"> O O </div>
            <div class="lcd__row" id="lcd-row-1"> </div>
          </div>
          <span class="lcd__label">LCD 1602A — 16×2 characters</span>
        </div>
      </div>

      <!-- Messages Card -->
      <div class="card animate-in" id="card-messages">
        <span class="card__label">Send Message</span>
        <p style="font-size: 13px; color: var(--text-muted); margin-bottom: 16px;">
          Send a text message to display on the LCD (max 16 chars, shown for 4 seconds).
        </p>
        <form class="message-form" id="message-form" onsubmit="return false;">
          <input type="text" class="message-input" id="message-input" placeholder="Type a message..." maxlength="16"
            autocomplete="off">
          <button type="submit" class="send-btn" id="send-btn">Send</button>
        </form>
      </div>

      <!-- Pomodoro Timer Card -->
      <div class="card animate-in" id="card-timer">
        <span class="card__label">Pomodoro Timer</span>
        <div class="timer">
          <div class="timer__display">
            <svg class="timer__ring" viewBox="0 0 200 200">
              <circle class="timer__ring-bg" cx="100" cy="100" r="90" />
              <circle class="timer__ring-progress" id="timer-progress" cx="100" cy="100" r="90"
                style="stroke-dasharray: 565.48; stroke-dashoffset: 565.48;" />
            </svg>
            <span class="timer__time" id="timer-time">25:00</span>
            <span class="timer__state" id="timer-state">Ready</span>
          </div>
          <div class="timer__controls">
            <button class="timer-btn timer-btn--primary" id="btn-start">▶ Start Focus</button>
            <button class="timer-btn" id="btn-pause" disabled>⏸ Pause</button>
            <button class="timer-btn timer-btn--danger" id="btn-stop" disabled>⏹ Stop</button>
            <button class="timer-btn" id="btn-break" disabled>☕ Break</button>
          </div>
        </div>
      </div>

      <!-- Serial Monitor Card -->
      <div class="card card--full animate-in" id="card-serial">
        <span class="card__label">Serial Monitor</span>
        <div class="serial-log" id="serial-log">
          <span class="serial-log__entry serial-log__entry--system">• Waiting for connection...</span>
        </div>
      </div>

    </div>

    <!-- ── Footer ──────────────────────────────────────── -->
    <footer class="footer">
      <p>
        TRENZIN — Arduino Nano + LCD 1602A
        · by <a href="https://magicoven.tech" target="_blank" rel="noopener">MagicOven</a>
      </p>
    </footer>

  </div>

  <!-- App Script -->
  <script src="app.js"></script>
</body>

</html>)=====";

const char WEB_CSS[] PROGMEM = R"=====(
/* ============================================================
   TRENZIN — Design System
   Premium dark mode with glassmorphism
   ============================================================ */

@import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&display=swap');

/* ── CSS Variables ──────────────────────────────────────── */
:root {
  --bg-primary: #09090b;
  --bg-secondary: #111114;
  --bg-surface: rgba(255, 255, 255, 0.035);
  --bg-surface-hover: rgba(255, 255, 255, 0.06);
  --bg-surface-active: rgba(255, 255, 255, 0.08);
  --bg-glass: rgba(17, 17, 20, 0.75);

  --border-subtle: rgba(255, 255, 255, 0.06);
  --border-default: rgba(255, 255, 255, 0.10);
  --border-strong: rgba(255, 255, 255, 0.16);

  --text-primary: #fafafa;
  --text-secondary: rgba(255, 255, 255, 0.72);
  --text-muted: rgba(255, 255, 255, 0.45);
  --text-ghost: rgba(255, 255, 255, 0.28);

  --accent: #32D74B;
  --accent-glow: rgba(50, 215, 75, 0.25);
  --accent-dim: rgba(50, 215, 75, 0.12);

  --danger: #FF453A;
  --warning: #FFD60A;
  --info: #64D2FF;

  --radius-sm: 10px;
  --radius-md: 16px;
  --radius-lg: 24px;
  --radius-full: 9999px;

  --transition-fast: 150ms cubic-bezier(0.25, 0.1, 0.25, 1);
  --transition-default: 250ms cubic-bezier(0.25, 0.1, 0.25, 1);
  --transition-slow: 400ms cubic-bezier(0.25, 0.1, 0.25, 1);

  --shadow-glow: 0 0 20px rgba(50, 215, 75, 0.15);
  --shadow-card: 0 2px 12px rgba(0, 0, 0, 0.4);
}

/* ── Reset & Base ───────────────────────────────────────── */
*,
*::before,
*::after {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

html {
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  width: 100%;
  max-width: 100vw;
  overflow-x: hidden;
}

body {
  font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
  background: var(--bg-primary);
  color: var(--text-primary);
  min-height: 100vh;
  line-height: 1.5;
  overflow-x: hidden;
}

/* Subtle background gradient */
body::before {
  content: '';
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  height: 60vh;
  background: radial-gradient(ellipse 80% 50% at 50% -10%, rgba(50, 215, 75, 0.06), transparent);
  pointer-events: none;
  z-index: 0;
}

/* ── App Container ──────────────────────────────────────── */
.app {
  position: relative;
  z-index: 1;
  max-width: 880px;
  margin: 0 auto;
  padding: 24px 20px 80px;
}

/* ── Header ─────────────────────────────────────────────── */
.header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 16px 0 40px;
}

.header__brand {
  display: flex;
  align-items: center;
  gap: 12px;
}

.header__logo {
  font-size: 28px;
  font-weight: 800;
  letter-spacing: -1.5px;
  color: var(--text-primary);
}

.header__logo span {
  color: var(--accent);
}

.header__tagline {
  font-size: 13px;
  color: var(--text-muted);
  font-weight: 500;
}

/* ── Connection Group ───────────────────────────────────── */
.connection-group {
  display: flex;
  align-items: center;
  gap: 12px;
}

.ip-input {
  width: 150px;
  padding: 10px 16px;
  border: 1px solid var(--border-default);
  border-radius: var(--radius-full);
  background: var(--bg-surface);
  color: var(--text-primary);
  font-family: inherit;
  font-size: 13px;
  font-weight: 500;
  outline: none;
  transition: all var(--transition-default);
  backdrop-filter: blur(12px);
}

.ip-input::placeholder {
  color: var(--text-ghost);
}

.ip-input:focus {
  border-color: rgba(50, 215, 75, 0.4);
  background: var(--bg-surface-hover);
  box-shadow: 0 0 0 3px var(--accent-dim);
}

/* ── Connection Button ──────────────────────────────────── */
.connect-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px 20px;
  border: 1px solid var(--border-default);
  border-radius: var(--radius-full);
  background: var(--bg-surface);
  color: var(--text-secondary);
  font-family: inherit;
  font-size: 13px;
  font-weight: 600;
  cursor: pointer;
  transition: all var(--transition-default);
  backdrop-filter: blur(12px);
}

.connect-btn:hover {
  background: var(--bg-surface-hover);
  border-color: var(--border-strong);
  color: var(--text-primary);
  transform: translateY(-1px);
}

.connect-btn--connected {
  border-color: rgba(50, 215, 75, 0.3);
  background: var(--accent-dim);
  color: var(--accent);
}

.connect-btn--connected:hover {
  background: rgba(50, 215, 75, 0.18);
  border-color: rgba(50, 215, 75, 0.45);
}

.connect-btn__dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: var(--text-ghost);
  transition: all var(--transition-default);
}

.connect-btn--connected .connect-btn__dot {
  background: var(--accent);
  box-shadow: 0 0 8px var(--accent-glow);
  animation: pulse-dot 2s ease-in-out infinite;
}

@keyframes pulse-dot {

  0%,
  100% {
    opacity: 1;
    box-shadow: 0 0 8px var(--accent-glow);
  }

  50% {
    opacity: 0.6;
    box-shadow: 0 0 16px var(--accent-glow);
  }
}

/* ── Section Layout ─────────────────────────────────────── */
.grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 16px;
}

@media (max-width: 700px) {
  .grid {
    grid-template-columns: 1fr;
  }
}

/* ── Card ───────────────────────────────────────────────── */
.card {
  background: var(--bg-surface);
  border: 1px solid var(--border-subtle);
  border-radius: var(--radius-lg);
  padding: 24px;
  backdrop-filter: blur(16px);
  transition: border-color var(--transition-default);
}

.card:hover {
  border-color: var(--border-default);
}

.card--full {
  grid-column: 1 / -1;
}

.card__label {
  display: block;
  font-size: 11px;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.12em;
  color: var(--text-ghost);
  margin-bottom: 16px;
}

.card__title {
  font-size: 20px;
  font-weight: 700;
  color: var(--text-primary);
  margin-bottom: 20px;
  letter-spacing: -0.3px;
}

/* ── Expression Buttons ─────────────────────────────────── */
.expressions-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 10px;
}

.expr-btn {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8px;
  padding: 18px 8px;
  border: 1px solid var(--border-subtle);
  border-radius: var(--radius-md);
  background: var(--bg-surface);
  color: var(--text-secondary);
  font-family: inherit;
  font-size: 12px;
  font-weight: 600;
  cursor: pointer;
  transition: all var(--transition-default);
}

.expr-btn:hover {
  background: var(--bg-surface-hover);
  border-color: var(--border-strong);
  color: var(--text-primary);
  transform: translateY(-2px);
  box-shadow: var(--shadow-card);
}

.expr-btn:active {
  transform: translateY(0);
}

.expr-btn--active {
  background: var(--accent-dim);
  border-color: rgba(50, 215, 75, 0.35);
  color: var(--accent);
  box-shadow: var(--shadow-glow);
}

.expr-btn__emoji {
  font-size: 28px;
  line-height: 1;
}

.expr-btn__label {
  text-transform: uppercase;
  letter-spacing: 0.06em;
}

/* ── Pomodoro Timer ─────────────────────────────────────── */
.timer {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 24px;
}

.timer__display {
  position: relative;
  width: 200px;
  height: 200px;
}

.timer__ring {
  width: 100%;
  height: 100%;
  transform: rotate(-90deg);
}

.timer__ring-bg {
  fill: none;
  stroke: rgba(255, 255, 255, 0.06);
  stroke-width: 6;
}

.timer__ring-progress {
  fill: none;
  stroke: var(--accent);
  stroke-width: 6;
  stroke-linecap: round;
  stroke-dasharray: 565.48;
  stroke-dashoffset: 0;
  transition: stroke-dashoffset 1s linear;
  filter: drop-shadow(0 0 8px var(--accent-glow));
}

.timer__ring-progress--break {
  stroke: var(--info);
  filter: drop-shadow(0 0 8px rgba(100, 210, 255, 0.25));
}

.timer__ring-progress--paused {
  stroke: var(--warning);
  filter: drop-shadow(0 0 8px rgba(255, 214, 10, 0.25));
  animation: pulse-ring 1.5s ease-in-out infinite;
}

@keyframes pulse-ring {

  0%,
  100% {
    opacity: 1;
  }

  50% {
    opacity: 0.5;
  }
}

.timer__time {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -55%);
  font-size: 42px;
  font-weight: 800;
  letter-spacing: -2px;
  color: var(--text-primary);
  font-variant-numeric: tabular-nums;
}

.timer__state {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, 28px);
  font-size: 11px;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.15em;
  color: var(--text-muted);
}

.timer__controls {
  display: flex;
  gap: 10px;
  flex-wrap: wrap;
  justify-content: center;
}

.timer-btn {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 10px 20px;
  border: 1px solid var(--border-default);
  border-radius: var(--radius-full);
  background: var(--bg-surface);
  color: var(--text-secondary);
  font-family: inherit;
  font-size: 13px;
  font-weight: 600;
  cursor: pointer;
  transition: all var(--transition-default);
}

.timer-btn:hover {
  background: var(--bg-surface-hover);
  border-color: var(--border-strong);
  color: var(--text-primary);
  transform: translateY(-1px);
}

.timer-btn--primary {
  background: var(--accent);
  border-color: var(--accent);
  color: #000;
}

.timer-btn--primary:hover {
  background: #3de058;
  border-color: #3de058;
  color: #000;
  box-shadow: var(--shadow-glow);
}

.timer-btn--danger {
  border-color: rgba(255, 69, 58, 0.3);
  color: var(--danger);
}

.timer-btn--danger:hover {
  background: rgba(255, 69, 58, 0.1);
  border-color: rgba(255, 69, 58, 0.5);
}

.timer-btn:disabled {
  opacity: 0.3;
  pointer-events: none;
}

/* ── LCD Preview ────────────────────────────────────────── */
.lcd-preview {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 16px;
}

.lcd {
  background: #1a3a1a;
  border: 3px solid #2a2a2a;
  border-radius: 12px;
  padding: 16px 20px;
  box-shadow:
    inset 0 2px 8px rgba(0, 0, 0, 0.5),
    0 4px 24px rgba(0, 0, 0, 0.3);
  width: 100%;
  max-width: 380px;
}

.lcd__row {
  font-family: 'Courier New', 'Consolas', monospace;
  font-size: 20px;
  font-weight: 700;
  letter-spacing: 7.5px;
  color: #7fff7f;
  text-shadow: 0 0 8px rgba(127, 255, 127, 0.4);
  line-height: 1.8;
  white-space: pre;
  min-height: 28px;
  overflow: hidden;
}

.lcd__label {
  font-size: 11px;
  font-weight: 600;
  color: var(--text-ghost);
  text-transform: uppercase;
  letter-spacing: 0.1em;
}

/* ── Messages Section ───────────────────────────────────── */
.message-form {
  display: flex;
  gap: 10px;
}

.message-input {
  flex: 1;
  padding: 12px 16px;
  border: 1px solid var(--border-default);
  border-radius: var(--radius-md);
  background: var(--bg-surface);
  color: var(--text-primary);
  font-family: inherit;
  font-size: 14px;
  font-weight: 500;
  outline: none;
  transition: all var(--transition-default);
}

.message-input::placeholder {
  color: var(--text-ghost);
}

.message-input:focus {
  border-color: rgba(50, 215, 75, 0.4);
  background: var(--bg-surface-hover);
  box-shadow: 0 0 0 3px var(--accent-dim);
}

.message-input:disabled {
  opacity: 0.4;
}

.send-btn {
  padding: 12px 22px;
  border: none;
  border-radius: var(--radius-md);
  background: var(--accent);
  color: #000;
  font-family: inherit;
  font-size: 14px;
  font-weight: 700;
  cursor: pointer;
  transition: all var(--transition-default);
  white-space: nowrap;
}

.send-btn:hover {
  background: #3de058;
  box-shadow: var(--shadow-glow);
  transform: translateY(-1px);
}

.send-btn:active {
  transform: translateY(0);
}

.send-btn:disabled {
  opacity: 0.3;
  pointer-events: none;
}

/* ── Log / Serial Monitor ──────────────────────────────── */
.serial-log {
  background: rgba(0, 0, 0, 0.35);
  border: 1px solid var(--border-subtle);
  border-radius: var(--radius-sm);
  padding: 12px 16px;
  max-height: 160px;
  overflow-y: auto;
  overflow-x: hidden;
  word-break: break-word;
  font-family: 'Courier New', 'Consolas', monospace;
  font-size: 12px;
  line-height: 1.7;
  color: var(--text-muted);
}

.serial-log::-webkit-scrollbar {
  width: 4px;
}

.serial-log::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.1);
  border-radius: 4px;
}

.serial-log__entry {
  display: block;
}

.serial-log__entry--rx {
  color: var(--accent);
}

.serial-log__entry--tx {
  color: var(--info);
}

.serial-log__entry--error {
  color: var(--danger);
}

.serial-log__entry--system {
  color: var(--text-ghost);
  font-style: italic;
}

/* ── Status Bar ─────────────────────────────────────────── */
.status-bar {
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 12px 0;
  margin-bottom: 16px;
  border-bottom: 1px solid var(--border-subtle);
}

.status-chip {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 4px 12px;
  border-radius: var(--radius-full);
  background: var(--bg-surface);
  border: 1px solid var(--border-subtle);
  font-size: 12px;
  font-weight: 600;
  color: var(--text-muted);
}

.status-chip--active {
  background: var(--accent-dim);
  border-color: rgba(50, 215, 75, 0.3);
  color: var(--accent);
}

.status-chip__icon {
  font-size: 14px;
}

/* ── Footer ─────────────────────────────────────────────── */
.footer {
  text-align: center;
  padding: 40px 0 0;
  font-size: 12px;
  color: var(--text-ghost);
}

.footer a {
  color: var(--text-muted);
  text-decoration: none;
  transition: color var(--transition-fast);
}

.footer a:hover {
  color: var(--accent);
}

/* ── Animations ─────────────────────────────────────────── */
@keyframes fadeInUp {
  from {
    opacity: 0;
    transform: translateY(20px);
  }

  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.animate-in {
  animation: fadeInUp 0.5s ease-out forwards;
}

.animate-in:nth-child(1) {
  animation-delay: 0.05s;
}

.animate-in:nth-child(2) {
  animation-delay: 0.1s;
}

.animate-in:nth-child(3) {
  animation-delay: 0.15s;
}

.animate-in:nth-child(4) {
  animation-delay: 0.2s;
}

.animate-in:nth-child(5) {
  animation-delay: 0.25s;
}

/* ── Responsive ─────────────────────────────────────────── */
@media (max-width: 480px) {
  .app {
    padding: 16px 14px 60px;
  }

  .header {
    flex-direction: column;
    gap: 16px;
    align-items: flex-start;
    padding-bottom: 28px;
  }

  .expressions-grid {
    grid-template-columns: repeat(3, 1fr);
    gap: 8px;
  }

  .expr-btn {
    padding: 14px 6px;
  }

  .expr-btn__emoji {
    font-size: 24px;
  }

  .timer__display {
    width: 170px;
    height: 170px;
  }

  .timer__time {
    font-size: 36px;
  }

  .lcd__row {
    font-size: 16px;
    letter-spacing: 6px;
  }

  .message-form {
    flex-direction: column;
  }

  .message-input {
    font-size: 16px;
  }

  .send-btn {
    width: 100%;
  }

  .card {
    padding: 16px;
  }
}

/* ── Disabled State Overlay ─────────────────────────────── */
.disabled-overlay {
  position: relative;
}

.disabled-overlay::after {
  content: 'Connect Arduino to use';
  position: absolute;
  inset: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(9, 9, 11, 0.7);
  backdrop-filter: blur(4px);
  border-radius: var(--radius-lg);
  font-size: 13px;
  font-weight: 600;
  color: var(--text-muted);
  opacity: 0;
  pointer-events: none;
  transition: opacity var(--transition-default);
}

.disabled-overlay--active::after {
  opacity: 1;
  pointer-events: auto;
})=====";

const char WEB_JS[] PROGMEM = R"=====(
// ============================================================
// TRENZIN — Web Serial Dashboard
// Communicates with Arduino Nano via Web Serial API
// ============================================================

// ── WebSocket Connection Manager ───────────────────────────
class WebSocketConnection {
  constructor() {
    this.socket = null;
    this.connected = false;
    this.onReceive = null;
    this.onDisconnect = null;
  }

  connect(ip) {
    return new Promise((resolve) => {
      try {
        this.socket = new WebSocket(`ws://${ip}:81/`);

        this.socket.onopen = () => {
          this.connected = true;
          resolve(true);
        };

        this.socket.onmessage = (event) => {
          if (this.onReceive) {
            const lines = event.data.split('\n');
            lines.forEach(line => {
              if (line.trim()) this.onReceive(line.trim());
            });
          }
        };

        this.socket.onclose = () => {
          this.connected = false;
          if (this.onDisconnect) this.onDisconnect();
          resolve(false);
        };

        this.socket.onerror = (err) => {
          console.error('WebSocket error:', err);
          resolve(false);
        };
      } catch (err) {
        console.error('Connection failed:', err);
        resolve(false);
      }
    });
  }

  async send(data) {
    if (!this.connected || !this.socket) return false;
    try {
      this.socket.send(data + '\n');
      return true;
    } catch (err) {
      console.error('Send error:', err);
      return false;
    }
  }

  async disconnect() {
    if (this.socket) {
      this.socket.close();
    }
  }
}

// ── Pomodoro Timer (local mirror) ──────────────────────────
class PomodoroTimer {
  constructor() {
    this.state = 'off'; // off, focus, break, paused, done
    this.totalSeconds = 0;
    this.remainingSeconds = 0;
    this.interval = null;
    this.onTick = null;
    this.onDone = null;
  }

  start(type = 'focus') {
    this.stop();
    this.totalSeconds = type === 'focus' ? 25 * 60 : 5 * 60;
    this.remainingSeconds = this.totalSeconds;
    this.state = type;
    this.interval = setInterval(() => this._tick(), 1000);
    if (this.onTick) this.onTick();
  }

  pause() {
    if (this.state === 'focus' || this.state === 'break') {
      clearInterval(this.interval);
      this.interval = null;
      this.state = 'paused';
      if (this.onTick) this.onTick();
    }
  }

  resume() {
    if (this.state === 'paused') {
      this.state = this.remainingSeconds > 5 * 60 ? 'focus' : 'break';
      this.interval = setInterval(() => this._tick(), 1000);
      if (this.onTick) this.onTick();
    }
  }

  stop() {
    clearInterval(this.interval);
    this.interval = null;
    this.state = 'off';
    this.remainingSeconds = 0;
    this.totalSeconds = 0;
    if (this.onTick) this.onTick();
  }

  _tick() {
    if (this.remainingSeconds > 0) {
      this.remainingSeconds--;
      if (this.onTick) this.onTick();
      if (this.remainingSeconds === 0) {
        this.state = 'done';
        clearInterval(this.interval);
        this.interval = null;
        if (this.onDone) this.onDone();
      }
    }
  }

  get minutes() {
    return Math.floor(this.remainingSeconds / 60);
  }

  get seconds() {
    return this.remainingSeconds % 60;
  }

  get progress() {
    if (this.totalSeconds === 0) return 0;
    return 1 - (this.remainingSeconds / this.totalSeconds);
  }

  get timeString() {
    return `${String(this.minutes).padStart(2, '0')}:${String(this.seconds).padStart(2, '0')}`;
  }
}

// ── Application State ──────────────────────────────────────
const trenzinConn = new WebSocketConnection();
const timer = new PomodoroTimer();
let currentExpression = 'idle';
let lcdRow0 = '                ';
let lcdRow1 = '                ';

// ── DOM References ─────────────────────────────────────────
const $ = (sel) => document.querySelector(sel);
const $$ = (sel) => document.querySelectorAll(sel);

// ── Initialize ─────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
  setupEventListeners();
  updateTimerDisplay();
  updateLCDFace(currentExpression);
  updateLCDPreview();
  updateConnectionUI(false);

  // Auto connect se estiver rodando no próprio ESP32 ou com IP via URL
  if (window.location.hostname && window.location.hostname !== 'localhost') {
    handleConnect();
  }
});

// ── Event Listeners ────────────────────────────────────────
function setupEventListeners() {
  // Connect button
  $('#connect-btn').addEventListener('click', handleConnect);

  // Expression buttons
  $$('.expr-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      const expr = btn.dataset.expr;
      sendExpression(expr);
    });
  });

  // Timer buttons
  $('#btn-start').addEventListener('click', () => {
    timer.start('focus');
    trenzinConn.send('TMR:START');
    addLog('TMR:START', 'tx');
  });

  $('#btn-pause').addEventListener('click', () => {
    if (timer.state === 'paused') {
      timer.resume();
      trenzinConn.send('TMR:RESUME');
      addLog('TMR:RESUME', 'tx');
    } else {
      timer.pause();
      trenzinConn.send('TMR:PAUSE');
      addLog('TMR:PAUSE', 'tx');
    }
  });

  $('#btn-stop').addEventListener('click', () => {
    timer.stop();
    trenzinConn.send('TMR:STOP');
    addLog('TMR:STOP', 'tx');
  });

  $('#btn-break').addEventListener('click', () => {
    timer.start('break');
    trenzinConn.send('TMR:BREAK');
    addLog('TMR:BREAK', 'tx');
  });

  // Message form
  $('#message-form').addEventListener('submit', (e) => {
    e.preventDefault();
    const input = $('#message-input');
    const msg = input.value.trim();
    if (msg) {
      trenzinConn.send('MSG:' + msg);
      addLog('MSG:' + msg, 'tx');
      lcdRow1 = msg.substring(0, 16).padEnd(16, ' ');
      updateLCDPreview();
      input.value = '';

      // Revert LCD preview after 4s (matching Arduino timeout)
      setTimeout(() => {
        updateLCDFromTimer();
        updateLCDPreview();
      }, 4000);
    }
  });

  // Timer callbacks
  timer.onTick = () => {
    updateTimerDisplay();
    updateLCDFromTimer();
    updateLCDPreview();
  };

  timer.onDone = () => {
    updateTimerDisplay();
    updateLCDFromTimer();
    updateLCDPreview();
    // Optional: browser notification
    if (Notification.permission === 'granted') {
      new Notification('Trenzin', {
        body: timer.totalSeconds > 5 * 60 ? '🎉 Focus session done!' : '☕ Break is over!',
        icon: 'data:image/svg+xml,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100"><text y="80" font-size="80">🤖</text></svg>'
      });
    }
  };
}

// ── Connection ─────────────────────────────────────────────
async function handleConnect() {
  if (trenzinConn.connected) {
    await trenzinConn.disconnect();
    updateConnectionUI(false);
    addLog('Disconnected', 'system');
    return;
  }

  const ip = window.location.hostname || '192.168.1.15'; // Fallback ip
  if (!ip) {
    addLog('Cannot determine IP address', 'error');
    return;
  }

  // Set up handlers before connecting so early messages aren't lost
  trenzinConn.onReceive = (data) => {
    addLog(data, 'rx');
    parseArduinoMessage(data);
  };

  trenzinConn.onDisconnect = () => {
    updateConnectionUI(false);
    addLog('Trenzin disconnected', 'error');
  };

  addLog('Connecting to WebSocket...', 'system');
  const success = await trenzinConn.connect(ip);

  if (success) {
    updateConnectionUI(true);
    addLog('Connected to Trenzin', 'system');

    // Request notification permission
    if (Notification.permission === 'default') {
      Notification.requestPermission();
    }
  } else {
    addLog('Connection failed', 'error');
  }
}

// ── Send Expression ────────────────────────────────────────
function sendExpression(expr) {
  currentExpression = expr;
  const cmd = 'EXP:' + expr.toUpperCase();
  trenzinConn.send(cmd);
  addLog(cmd, 'tx');

  // Update UI
  $$('.expr-btn').forEach(btn => {
    btn.classList.toggle('expr-btn--active', btn.dataset.expr === expr);
  });

  // Update LCD preview
  updateLCDFace(expr);
  updateLCDPreview();
}

// ── Parse Arduino Messages ─────────────────────────────────
function parseArduinoMessage(data) {
  if (data.startsWith('STATE:')) {
    const payload = data.substring(6);

    if (payload === 'IDLE') {
      // Timer idle
    } else if (payload === 'DONE') {
      timer.state = 'done';
      updateTimerDisplay();
    } else if (payload.startsWith('FOCUS:') || payload.startsWith('BREAK:')) {
      // Sync timer from Arduino
      const parts = payload.split(':');
      if (parts.length >= 3) {
        const mins = parseInt(parts[1]);
        const secs = parseInt(parts[2]);
        const totalRemaining = mins * 60 + secs;

        // Only sync if significantly different (>2s drift)
        if (Math.abs(timer.remainingSeconds - totalRemaining) > 2) {
          timer.remainingSeconds = totalRemaining;
          updateTimerDisplay();
        }
      }
    }
  } else if (data.startsWith('FACE:')) {
    const expr = data.substring(5).toLowerCase();
    currentExpression = expr;
    $$('.expr-btn').forEach(btn => {
      btn.classList.toggle('expr-btn--active', btn.dataset.expr === expr);
    });
    updateLCDFace(expr);
    updateLCDPreview();
  } else if (data.startsWith('ACK:')) {
    // Acknowledged, no action needed
  }
}

// ── UI Updates ─────────────────────────────────────────────
function updateConnectionUI(connected) {
  const btn = $('#connect-btn');
  const dot = btn.querySelector('.connect-btn__dot');
  const text = btn.querySelector('.connect-btn__text');

  if (connected) {
    btn.classList.add('connect-btn--connected');
    text.textContent = 'Connected';
  } else {
    btn.classList.remove('connect-btn--connected');
    text.textContent = 'Connect Arduino';
  }

  // Enable/disable controls
  const overlay = $$('.disabled-overlay');
  overlay.forEach(el => {
    el.classList.toggle('disabled-overlay--active', !connected);
  });

  // Status chips
  $('#status-connection').classList.toggle('status-chip--active', connected);
  $('#status-connection').querySelector('.status-chip__text').textContent =
    connected ? 'Online' : 'Offline';
}

function updateTimerDisplay() {
  const timeEl = $('#timer-time');
  const stateEl = $('#timer-state');
  const progressEl = $('#timer-progress');
  const circumference = 2 * Math.PI * 90; // r=90

  // Time display
  if (timer.state === 'off') {
    timeEl.textContent = '25:00';
    stateEl.textContent = 'Ready';
  } else if (timer.state === 'done') {
    timeEl.textContent = '00:00';
    stateEl.textContent = 'Done!';
  } else {
    timeEl.textContent = timer.timeString;
    if (timer.state === 'paused') {
      stateEl.textContent = 'Paused';
    } else if (timer.state === 'focus') {
      stateEl.textContent = 'Focus';
    } else if (timer.state === 'break') {
      stateEl.textContent = 'Break';
    }
  }

  // Ring progress
  const offset = circumference * (1 - timer.progress);
  progressEl.style.strokeDashoffset = offset;

  // Ring color classes
  progressEl.classList.remove('timer__ring-progress--break', 'timer__ring-progress--paused');
  if (timer.state === 'break') {
    progressEl.classList.add('timer__ring-progress--break');
  } else if (timer.state === 'paused') {
    progressEl.classList.add('timer__ring-progress--paused');
  }

  // Button states
  const isRunning = timer.state === 'focus' || timer.state === 'break';
  const isPaused = timer.state === 'paused';
  const isDone = timer.state === 'done';
  const isOff = timer.state === 'off';

  $('#btn-start').disabled = isRunning || isPaused;
  $('#btn-pause').disabled = isOff || isDone;
  $('#btn-stop').disabled = isOff;
  $('#btn-break').disabled = isRunning || isPaused;

  // Pause button text
  $('#btn-pause').textContent = isPaused ? '▶ Resume' : '⏸ Pause';
}

function updateLCDFace(expr) {
  const faceMap = {
    idle: '     O    O     ',
    happy: '     ^ vv ^     ',
    sad: '     T    T     ',
    angry: '     >·><·<     ',
    focus: '     =    =     ',
    sleep: '    - _- z Z    ',
  };
  lcdRow0 = faceMap[expr] || faceMap.idle;
}

function updateLCDFromTimer() {
  if (timer.state === 'off') {
    lcdRow1 = '                ';
  } else if (timer.state === 'focus') {
    lcdRow1 = ` FOCUS  ${timer.timeString}   `;
  } else if (timer.state === 'break') {
    lcdRow1 = ` BREAK  ${timer.timeString}   `;
  } else if (timer.state === 'paused') {
    lcdRow1 = ` PAUSED ${timer.timeString}   `;
  } else if (timer.state === 'done') {
    lcdRow1 = '   DONE! :D     ';
  }
  lcdRow1 = lcdRow1.substring(0, 16).padEnd(16, ' ');
}

function updateLCDPreview() {
  $('#lcd-row-0').textContent = lcdRow0.substring(0, 16);
  $('#lcd-row-1').textContent = lcdRow1.substring(0, 16);
}

// ── Serial Log ─────────────────────────────────────────────
function addLog(message, type = 'system') {
  const log = $('#serial-log');
  const entry = document.createElement('span');
  entry.className = `serial-log__entry serial-log__entry--${type}`;

  const prefix = {
    rx: '← ',
    tx: '→ ',
    error: '✕ ',
    system: '• '
  };

  const time = new Date().toLocaleTimeString('en-US', {
    hour12: false, hour: '2-digit', minute: '2-digit', second: '2-digit'
  });

  entry.textContent = `[${time}] ${prefix[type] || ''}${message}\n`;
  log.appendChild(entry);
  log.scrollTop = log.scrollHeight;

  // Keep only last 100 entries
  while (log.children.length > 100) {
    log.removeChild(log.firstChild);
  }
}
)=====";

#endif
