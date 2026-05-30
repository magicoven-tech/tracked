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

  <!-- Fonts & Icons -->
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&display=swap" rel="stylesheet">
  <script src="https://unpkg.com/lucide@latest"></script>

  <!-- Styles -->
  <link rel="stylesheet" href="style.css">

  <!-- Favicon -->
  <link rel="icon"
    href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='80' font-size='80'>🤖</text></svg>">
</head>

<body>
  <div class="app">

    <!-- ── VIEW: HOME ──────────────────────────────────────── -->
    <div class="view view--active" id="view-home">
      <header class="header animate-in" id="header">
        <div class="header__brand">
          <h1 class="header__logo">TREN<span>ZIN</span></h1>
          <span class="header__tagline" id="date-display">Seu companheiro de mesa</span>
        </div>
        <div class="connection-group">
          <button class="connect-btn" id="connect-btn">
            <span class="connect-btn__dot"></span>
            <span class="connect-btn__text">Connect Trenzin</span>
          </button>
        </div>
      </header>

      <div class="status-bar animate-in" id="status-bar">
        <div class="status-chip" id="status-connection">
          <span class="status-chip__icon"><i data-lucide="zap" width="16" height="16"></i></span>
          <span class="status-chip__text">Offline</span>
        </div>

      </div>

      <!-- Minimalist LCD Preview -->
      <div class="lcd-minimalist animate-in" id="lcd-minimalist">
        <div class="lcd__row" id="lcd-row-0"> O O </div>
        <div class="lcd__row" id="lcd-row-1"> </div>
      </div>

      <!-- Home Grid Shortcuts -->
      <div class="home-shortcuts animate-in">
        <button class="shortcut-btn" onclick="navigateTo('expressions')">
          <span class="shortcut-btn__icon"><i data-lucide="smile" width="32" height="32"></i></span>
          <span class="shortcut-btn__label">Expressões</span>
        </button>
        <button class="shortcut-btn" onclick="navigateTo('messages')">
          <span class="shortcut-btn__icon"><i data-lucide="message-square" width="32" height="32"></i></span>
          <span class="shortcut-btn__label">Mensagem</span>
        </button>
        <button class="shortcut-btn" onclick="navigateTo('timer')">
          <span class="shortcut-btn__icon"><i data-lucide="timer" width="32" height="32"></i></span>
          <span class="shortcut-btn__label">Pomodoro</span>
        </button>
        <button class="shortcut-btn" onclick="navigateTo('alarm')">
          <span class="shortcut-btn__icon"><i data-lucide="alarm-clock" width="32" height="32"></i></span>
          <span class="shortcut-btn__label">Alarme</span>
        </button>
        <button class="shortcut-btn" onclick="navigateTo('serial')">
          <span class="shortcut-btn__icon"><i data-lucide="terminal" width="32" height="32"></i></span>
          <span class="shortcut-btn__label">Serial Monitor</span>
        </button>
        <button class="shortcut-btn" onclick="navigateTo('settings')">
          <span class="shortcut-btn__icon"><i data-lucide="settings" width="32" height="32"></i></span>
          <span class="shortcut-btn__label">Configurações</span>
        </button>
      </div>
    </div>

    <!-- ── VIEW: EXPRESSIONS ───────────────────────────────── -->
    <div class="view" id="view-expressions">
      <div class="view-header">
        <button class="back-btn" onclick="navigateTo('home')">← Voltar</button>
        <h2>Expressões</h2>
      </div>
      <div class="card animate-in">
        <div class="expressions-grid">
          <button class="expr-btn expr-btn--active" data-expr="idle" id="expr-idle">
            <span class="expr-btn__emoji"><i data-lucide="smile" width="36" height="36"></i></span>
            <span class="expr-btn__label">Idle</span>
          </button>
          <button class="expr-btn" data-expr="happy" id="expr-happy">
            <span class="expr-btn__emoji"><i data-lucide="laugh" width="36" height="36"></i></span>
            <span class="expr-btn__label">Happy</span>
          </button>
          <button class="expr-btn" data-expr="sad" id="expr-sad">
            <span class="expr-btn__emoji"><i data-lucide="frown" width="36" height="36"></i></span>
            <span class="expr-btn__label">Sad</span>
          </button>
          <button class="expr-btn" data-expr="angry" id="expr-angry">
            <span class="expr-btn__emoji"><i data-lucide="angry" width="36" height="36"></i></span>
            <span class="expr-btn__label">Angry</span>
          </button>
          <button class="expr-btn" data-expr="focus" id="expr-focus">
            <span class="expr-btn__emoji"><i data-lucide="target" width="36" height="36"></i></span>
            <span class="expr-btn__label">Focus</span>
          </button>
          <button class="expr-btn" data-expr="sleep" id="expr-sleep">
            <span class="expr-btn__emoji"><i data-lucide="moon" width="36" height="36"></i></span>
            <span class="expr-btn__label">Sleep</span>
          </button>
        </div>
      </div>
    </div>

    <!-- ── VIEW: MESSAGES ──────────────────────────────────── -->
    <div class="view" id="view-messages">
      <div class="view-header">
        <button class="back-btn" onclick="navigateTo('home')">← Voltar</button>
        <h2>Enviar Mensagem</h2>
      </div>
      <div class="card animate-in">
        <p style="font-size: 14px; color: var(--text-muted); margin-bottom: 24px;">
          Digite uma mensagem curta para ser exibida no visor do Trenzin.
        </p>
        <form class="message-form" id="message-form" onsubmit="return false;">
          <input type="text" class="message-input" id="message-input" placeholder="Sua mensagem..." maxlength="16"
            autocomplete="off">
          <button type="submit" class="send-btn" id="send-btn">Enviar</button>
        </form>
      </div>
    </div>

    <!-- ── VIEW: POMODORO ──────────────────────────────────── -->
    <div class="view" id="view-timer">
      <div class="view-header" style="flex-direction: row; justify-content: space-between; align-items: flex-end;">
        <div>
          <button class="back-btn" onclick="navigateTo('home')">← Voltar</button>
          <h2>Pomodoro</h2>
        </div>
        <button class="icon-btn" id="btn-pomodoro-settings">
          <i data-lucide="settings" width="24" height="24"></i>
        </button>
      </div>
      <div class="card animate-in">
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
            <button class="timer-btn timer-btn--primary" id="btn-start"><i data-lucide="play" width="16"
                height="16"></i> Iniciar foco</button>
            <button class="timer-btn" id="btn-pause" disabled><i data-lucide="pause" width="16" height="16"></i>
              Pausar</button>
            <button class="timer-btn timer-btn--danger" id="btn-stop" disabled><i data-lucide="square" width="16"
                height="16"></i> Parar</button>
            <button class="timer-btn" id="btn-break" disabled><i data-lucide="coffee" width="16" height="16"></i>
              Pausa</button>
          </div>
        </div>
      </div>
    </div>

    <!-- ── VIEW: ALARM ─────────────────────────────────────── -->
    <div class="view" id="view-alarm">
      <div class="view-header" style="flex-direction: row; justify-content: space-between; align-items: flex-end;">
        <div>
          <button class="back-btn" onclick="navigateTo('home')">← Voltar</button>
          <h2>Alarme</h2>
        </div>
        <button class="icon-btn" id="btn-alarm-add">
          <i data-lucide="plus" width="24" height="24"></i>
        </button>
      </div>

      <div class="alarms-wrapper animate-in">
        <div class="alarms-list" id="alarms-list">
          <!-- Os cards de alarme serão injetados via JS -->
        </div>
      </div>
    </div>

    <!-- ── VIEW: SERIAL MONITOR ────────────────────────────── -->
    <div class="view" id="view-serial">
      <div class="view-header">
        <button class="back-btn" onclick="navigateTo('home')">← Voltar</button>
        <h2>Monitor Serial</h2>
      </div>
      <div class="card card--full animate-in" style="flex: 1; display: flex; flex-direction: column;">
        <div class="serial-log" id="serial-log" style="flex: 1;">
          <span class="serial-log__entry serial-log__entry--system">• Aguardando conexão...</span>
        </div>
      </div>
    </div>

    <!-- ── VIEW: SETTINGS ──────────────────────────────────── -->
    <div class="view" id="view-settings">
      <div class="view-header">
        <button class="back-btn" onclick="navigateTo('home')">← Voltar</button>
        <h2>Configurações</h2>
      </div>
      <div class="card animate-in">
        <p style="font-size: 14px; color: var(--text-muted); text-align: center; padding: 40px 0;">
          Configurações do dispositivo em breve. <i data-lucide="settings" width="20" height="20"
            style="vertical-align: middle; margin-left: 4px;"></i>
        </p>
      </div>
    </div>

    <!-- ── Footer ──────────────────────────────────────── -->
    <footer class="footer">
      <p>
        TRENZIN — ESP32 + LCD 1602A
        · by <a href="https://magicoven.tech" target="_blank" rel="noopener">MagicOven</a>
      </p>
    </footer>

  </div>

  <!-- ── POMODORO SETTINGS MODAL ───────────────────────── -->
  <div class="modal-overlay" id="pomodoro-modal">
    <div class="modal">
      <div class="modal-header">
        <div style="width: 24px;"></div> <!-- Spacer for centering -->
        <h3 class="modal-title">CONFIGURAÇÃO</h3>
        <button class="modal-close" id="btn-close-modal">
          <i data-lucide="x" width="20" height="20"></i>
        </button>
      </div>

      <div class="modal-body">
        <div class="modal-section-title">
          <i data-lucide="clock" width="16" height="16"></i>
          <span>TEMPORIZADOR</span>
        </div>

        <h4 class="modal-label">Tempo (minutos)</h4>
        <div class="modal-grid-3">
          <div class="input-group">
            <label>Pomodoro</label>
            <input type="number" id="input-focus-time" value="25" min="1" max="90">
          </div>
          <div class="input-group">
            <label>Pausa curta</label>
            <input type="number" id="input-short-break" value="5" min="1" max="30">
          </div>
          <div class="input-group">
            <label>Pausa longa</label>
            <input type="number" id="input-long-break" value="15" min="1" max="60">
          </div>
        </div>

        <div class="modal-row" style="margin-top: 24px; margin-bottom: 24px;">
          <h4 class="modal-label" style="margin: 0;">Intervalo longo</h4>
          <input type="number" class="input-small" id="input-long-interval" value="4" min="1" max="10">
        </div>

        <button class="save-btn" id="btn-save-pomodoro">Salvar</button>
      </div>
    </div>
  </div>

  <!-- ── ALARM SETTINGS MODAL ───────────────────────── -->
  <div class="modal-overlay" id="alarm-modal">
    <div class="modal">
      <div class="modal-header">
        <div style="width: 24px;"></div> <!-- Spacer for centering -->
        <h3 class="modal-title">CONFIGURAÇÃO</h3>
        <button class="modal-close" id="btn-close-alarm-modal">
          <i data-lucide="x" width="20" height="20"></i>
        </button>
      </div>
      <div class="modal-body">
        <div class="modal-section-title">
          <i data-lucide="bell" width="16" height="16"></i>
          <span>ALARME</span>
        </div>

        <div class="input-group" style="margin-bottom: 20px;">
          <label>Nome do Alarme</label>
          <input type="text" id="input-alarm-label" placeholder="Ex: Acordar" maxlength="15" autocomplete="off">
        </div>

        <h4 class="modal-label">Hora do alarme</h4>
        <div style="display: flex; gap: 16px; align-items: flex-end; margin-bottom: 24px;">
          <div class="input-group" style="flex: 1;">
            <label>Horas</label>
            <input type="number" id="input-alarm-h" min="0" max="23" value="07">
          </div>
          <div style="font-size: 24px; font-weight: 600; padding-bottom: 8px; color: var(--text-muted);">:</div>
          <div class="input-group" style="flex: 1;">
            <label>Minutos</label>
            <input type="number" id="input-alarm-m" min="0" max="59" value="00">
          </div>
        </div>

        <div style="display: flex; gap: 12px; margin-top: 12px;">
          <button class="save-btn" id="btn-delete-alarm" style="background: var(--danger); width: auto; padding: 0 20px; display: none;">
            <i data-lucide="trash-2" width="20" height="20"></i>
          </button>
          <button class="save-btn" id="btn-save-alarm" style="flex: 1;">Salvar</button>
        </div>
      </div>
    </div>
  </div>

  <!-- Lucide Icons & JS -->
  <script src="app.js"></script>
  <script>
    lucide.createIcons();
  </script>
</body>

</html>)=====";

const char WEB_CSS[] PROGMEM = R"=====(
/* ============================================================
   TRENZIN — Design System
   Braun / Minimalist Dark Mode SPA
   ============================================================ */

@import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&display=swap');

/* ── CSS Variables ──────────────────────────────────────── */
:root {
  --bg-primary: #363945;
  --bg-surface: #464a59;
  --bg-surface-hover: #505566;
  --bg-surface-active: #5c6275;

  --border-subtle: transparent;
  --border-default: transparent;
  --border-strong: rgba(255, 255, 255, 0.1);

  --text-primary: #ffffff;
  --text-secondary: rgba(255, 255, 255, 0.7);
  --text-muted: rgba(255, 255, 255, 0.5);
  --text-ghost: rgba(255, 255, 255, 0.3);

  --accent: #ffffff;
  --accent-glow: rgba(255, 255, 255, 0.1);
  --accent-dim: rgba(255, 255, 255, 0.15);

  --danger: #FF453A;
  --warning: #FFD60A;
  --info: #64D2FF;
  --success: #32D74B;

  --radius-sm: 12px;
  --radius-md: 20px;
  --radius-lg: 28px;
  --radius-full: 9999px;

  --transition-fast: 150ms cubic-bezier(0.25, 0.1, 0.25, 1);
  --transition-default: 250ms cubic-bezier(0.25, 0.1, 0.25, 1);
  --transition-slow: 400ms cubic-bezier(0.25, 0.1, 0.25, 1);
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

/* ── App Container ──────────────────────────────────────── */
.app {
  position: relative;
  z-index: 1;
  max-width: 600px;
  /* Mais estreito para focar na proporção vertical/mobile */
  margin: 0 auto;
  padding: 40px 24px 80px;
}

/* ── SPA Views ──────────────────────────────────────────── */
.view {
  display: none;
  flex-direction: column;
  animation: fadeInView 0.3s ease-out forwards;
}

.view--active {
  display: flex;
}

@keyframes fadeInView {
  from {
    opacity: 0;
    transform: scale(0.98) translateY(10px);
  }

  to {
    opacity: 1;
    transform: scale(1) translateY(0);
  }
}

.view-header {
  display: flex;
  flex-direction: column;
  gap: 12px;
  margin-bottom: 32px;
}

.back-btn {
  background: none;
  border: none;
  color: var(--text-muted);
  font-family: inherit;
  font-size: 15px;
  font-weight: 600;
  cursor: pointer;
  align-self: flex-start;
  padding: 8px 12px 8px 0;
  transition: color var(--transition-fast);
}

.back-btn:hover {
  color: var(--text-primary);
}

.icon-btn {
  background: transparent;
  border: none;
  color: #fff;
  cursor: pointer;
  padding: 8px;
  border-radius: var(--radius-sm);
  display: flex;
  align-items: center;
  justify-content: center;
  transition: opacity 0.2s;
}

.icon-btn:hover {
  opacity: 0.8;
}

.view-header h2 {
  font-size: 32px;
  font-weight: 700;
  letter-spacing: -1px;
}

/* ── Header (Home) ──────────────────────────────────────── */
.header {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  margin-bottom: 24px;
}

.header__brand {
  display: flex;
  flex-direction: column;
}

.header__logo {
  font-size: 36px;
  font-weight: 700;
  letter-spacing: -1.2px;
  color: var(--text-primary);
  line-height: 1.1;
}

.header__logo span {
  font-weight: 400;
}

.header__tagline {
  font-size: 15px;
  color: var(--text-muted);
  font-weight: 500;
  margin-top: 4px;
}

/* ── Connection Group ───────────────────────────────────── */
.connection-group {
  display: flex;
  align-items: center;
  gap: 12px;
}

.connect-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px 16px;
  border: none;
  border-radius: var(--radius-full);
  background: var(--bg-surface);
  color: var(--text-primary);
  font-family: inherit;
  font-size: 13px;
  font-weight: 600;
  cursor: pointer;
  transition: all var(--transition-default);
}

.connect-btn:hover {
  background: var(--bg-surface-hover);
  transform: scale(0.97);
}

.connect-btn--connected {
  background: var(--accent-dim);
  color: var(--text-primary);
}

.connect-btn__dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: var(--text-ghost);
  transition: all var(--transition-default);
}

.connect-btn--connected .connect-btn__dot {
  background: var(--success);
}

/* ── Status Bar (Home) ──────────────────────────────────── */
.status-bar {
  display: flex;
  align-items: center;
  gap: 12px;
  margin-bottom: 40px;
}

.status-chip {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 6px 14px;
  border-radius: var(--radius-full);
  background: var(--bg-surface);
  font-size: 13px;
  font-weight: 600;
  color: var(--text-muted);
}

.status-chip--active {
  background: var(--bg-surface-active);
  color: var(--text-primary);
}

/* ── Minimalist LCD Preview (Home) ──────────────────────── */
.lcd-minimalist {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 60px 0 80px;
}

.lcd__row {
  font-family: inherit;
  font-size: 20px;
  font-weight: 600;
  letter-spacing: 8px;
  color: var(--text-primary);
  line-height: 1.6;
  white-space: pre;
  min-height: 32px;
  text-align: center;
}

/* ── Home Shortcuts Grid ────────────────────────────────── */
.home-shortcuts {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 16px;
}

.shortcut-btn {
  aspect-ratio: 1;
  /* Makes them perfectly square */
  background: var(--bg-surface);
  border: none;
  border-radius: var(--radius-md);
  padding: 20px;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  align-items: flex-start;
  color: var(--text-primary);
  font-family: inherit;
  cursor: pointer;
  transition: all var(--transition-fast);
}

.shortcut-btn:hover {
  background: var(--bg-surface-hover);
  transform: scale(0.97);
}

.shortcut-btn:active {
  transform: scale(0.94);
}

.shortcut-btn__icon {
  font-size: 28px;
}

.shortcut-btn__label {
  font-size: 13px;
  font-weight: 500;
  color: var(--text-secondary);
  text-align: left;
  line-height: 1.2;
}

/* ── Generic Card (Para as outras views) ────────────────── */
.card {
  background: transparent;
  /* No Braun style, the view itself is clean */
}

.card--full {
  flex: 1;
}

/* ── Expression Buttons ─────────────────────────────────── */
.expressions-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 16px;
}

.expr-btn {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 12px;
  padding: 24px 16px;
  border: none;
  border-radius: var(--radius-md);
  background: var(--bg-surface);
  color: var(--text-secondary);
  font-family: inherit;
  font-size: 14px;
  font-weight: 600;
  cursor: pointer;
  transition: all var(--transition-default);
}

.expr-btn:hover {
  background: var(--bg-surface-hover);
  color: var(--text-primary);
  transform: scale(0.97);
}

.expr-btn:active {
  transform: scale(0.94);
}

.expr-btn--active {
  background: var(--text-primary);
  color: var(--bg-primary);
}

.expr-btn__emoji {
  font-size: 36px;
  line-height: 1;
}

.expr-btn__label {
  text-transform: capitalize;
  letter-spacing: 0.02em;
}

/* ── Pomodoro Timer ─────────────────────────────────────── */
.timer {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 40px;
  padding-top: 20px;
}

.timer__display {
  position: relative;
  width: 260px;
  height: 260px;
}

.timer__ring {
  width: 100%;
  height: 100%;
  transform: rotate(-90deg);
}

.timer__ring-bg {
  fill: none;
  stroke: var(--bg-surface);
  stroke-width: 8;
}

.timer__ring-progress {
  fill: none;
  stroke: var(--text-primary);
  stroke-width: 8;
  stroke-linecap: round;
  stroke-dasharray: 565.48;
  stroke-dashoffset: 0;
  transition: stroke-dashoffset 1s linear;
}

.timer__ring-progress--break {
  stroke: var(--info);
}

.timer__ring-progress--paused {
  stroke: var(--warning);
}

.timer__time {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -55%);
  font-size: 56px;
  font-weight: 800;
  letter-spacing: -2px;
  color: var(--text-primary);
  font-variant-numeric: tabular-nums;
}

.timer__state {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, 40px);
  font-size: 13px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.1em;
  color: var(--text-muted);
}

.timer__controls {
  display: flex;
  gap: 16px;
  flex-wrap: wrap;
  justify-content: center;
}

.timer-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 16px 28px;
  border: none;
  border-radius: var(--radius-full);
  background: var(--bg-surface);
  color: var(--text-primary);
  font-family: inherit;
  font-size: 15px;
  font-weight: 600;
  cursor: pointer;
  transition: all var(--transition-default);
}

.timer-btn:hover {
  background: var(--bg-surface-hover);
  transform: scale(0.96);
}

.timer-btn--primary {
  background: var(--text-primary);
  color: var(--bg-primary);
}

.timer-btn--primary:hover {
  background: #e0e0e0;
}

.timer-btn--danger {
  color: var(--danger);
}

.timer-btn:disabled {
  opacity: 0.3;
  pointer-events: none;
}

/* ── Messages Section ───────────────────────────────────── */
.message-form {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.message-input {
  padding: 20px;
  border: none;
  border-radius: var(--radius-md);
  background: var(--bg-surface);
  color: var(--text-primary);
  font-family: inherit;
  font-size: 18px;
  font-weight: 500;
  outline: none;
  transition: all var(--transition-default);
}

.message-input::placeholder {
  color: var(--text-ghost);
}

.message-input:focus {
  background: var(--bg-surface-active);
}

.message-input:disabled {
  opacity: 0.4;
}

.send-btn {
  padding: 20px;
  border: none;
  border-radius: var(--radius-md);
  background: var(--text-primary);
  color: var(--bg-primary);
  font-family: inherit;
  font-size: 16px;
  font-weight: 700;
  cursor: pointer;
  transition: all var(--transition-default);
}

.send-btn:hover {
  background: #e0e0e0;
  transform: scale(0.98);
}

.send-btn:disabled {
  opacity: 0.3;
  pointer-events: none;
}

/* ── Log / Serial Monitor ──────────────────────────────── */
.serial-log {
  background: var(--bg-surface);
  border-radius: var(--radius-md);
  padding: 20px;
  height: 400px;
  /* Fixed height for log */
  overflow-y: auto;
  overflow-x: hidden;
  word-break: break-word;
  font-family: 'Courier New', 'Consolas', monospace;
  font-size: 13px;
  line-height: 1.8;
  color: var(--text-secondary);
}

.serial-log::-webkit-scrollbar {
  width: 6px;
}

.serial-log::-webkit-scrollbar-thumb {
  background: var(--bg-surface-active);
  border-radius: 6px;
}

.serial-log__entry {
  display: block;
}

.serial-log__entry--rx {
  color: var(--success);
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

/* ── Footer ─────────────────────────────────────────────── */
.footer {
  text-align: center;
  padding: 40px 0 0;
  font-size: 13px;
  color: var(--text-ghost);
}

.footer a {
  color: var(--text-muted);
  text-decoration: none;
  transition: color var(--transition-fast);
}

.footer a:hover {
  color: var(--text-primary);
}

/* ── Alarms List ─────────────────────────────────────────── */
.alarms-wrapper {
  margin: 0 -24px;
  padding: 0 24px;
}

.alarms-list {
  display: flex;
  gap: 16px;
  overflow-x: auto;
  padding-bottom: 24px;
  scroll-snap-type: x mandatory;
}

.alarms-list::-webkit-scrollbar {
  display: none;
}

.alarm-card {
  flex: 0 0 150px;
  height: 170px;
  scroll-snap-align: start;
  background: var(--surface);
  border-radius: 24px;
  padding: 20px;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  border: 1px solid rgba(255, 255, 255, 0.05);
  transition: all 0.3s;
  cursor: pointer;
}

.alarm-card--on {
  background: var(--surface-light);
  border-color: var(--primary);
}

.alarm-toggle-btn {
  width: 44px;
  height: 44px;
  border-radius: 50%;
  background: var(--surface-light);
  color: var(--text-muted);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 13px;
  font-weight: 700;
  cursor: pointer;
  border: none;
  transition: all 0.2s;
}

.alarm-card--on .alarm-toggle-btn {
  background: #ffffff;
  color: #363945;
  box-shadow: none;
}

.alarm-card-bottom {
  display: flex;
  flex-direction: column;
}

.alarm-card-label {
  font-size: 13px;
  color: var(--text-muted);
  margin-bottom: 4px;
}

.alarm-card-time {
  font-size: 38px;
  font-weight: 500;
  letter-spacing: -2px;
  color: var(--text-main);
}

/* ── Responsive ─────────────────────────────────────────── */
@media (max-width: 480px) {
  .app {
    padding: 30px 20px 60px;
  }

  .home-shortcuts {
    grid-template-columns: repeat(2, 1fr);
  }

  .expressions-grid {
    grid-template-columns: 1fr;
  }

  .timer__display {
    width: 220px;
    height: 220px;
  }

  .timer__time {
    font-size: 48px;
  }

  .lcd__row {
    font-size: 16px;
    letter-spacing: 4px;
  }
}

/* ── Disabled State Overlay ─────────────────────────────── */
.disabled-overlay {
  position: relative;
}

.disabled-overlay::after {
  content: 'Conecte o Trenzin';
  position: absolute;
  inset: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(54, 57, 69, 0.85);
  /* Coincide com bg-primary */
  backdrop-filter: blur(4px);
  border-radius: var(--radius-md);
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
  opacity: 0;
  pointer-events: none;
  transition: opacity var(--transition-default);
}

.disabled-overlay--active::after {
  opacity: 1;
  pointer-events: auto;
}

/* ── Pomodoro Modal ─────────────────────────────────────── */
.modal-overlay {
  position: fixed;
  inset: 0;
  background: rgba(0, 0, 0, 0.6);
  backdrop-filter: blur(8px);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
  opacity: 0;
  pointer-events: none;
  transition: opacity var(--transition-default);
}

.modal-overlay.active {
  opacity: 1;
  pointer-events: auto;
}

.modal {
  background: var(--bg-primary);
  border: 1px solid var(--border-strong);
  border-radius: var(--radius-lg);
  width: 90%;
  max-width: 400px;
  padding: 24px;
  transform: translateY(20px);
  transition: transform var(--transition-default);
  box-shadow: 0 10px 40px rgba(0, 0, 0, 0.4);
}

.modal-overlay.active .modal {
  transform: translateY(0);
}

.modal-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 24px;
  padding-bottom: 16px;
  border-bottom: 1px solid var(--border-strong);
}

.modal-title {
  font-size: 14px;
  font-weight: 700;
  color: var(--text-secondary);
  letter-spacing: 0.05em;
}

.modal-close {
  background: none;
  border: none;
  color: var(--text-muted);
  cursor: pointer;
  transition: color var(--transition-fast);
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 4px;
}

.modal-close:hover {
  color: var(--text-primary);
}

.modal-section-title {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 13px;
  font-weight: 700;
  color: var(--text-muted);
  letter-spacing: 0.05em;
  margin-bottom: 24px;
}

.modal-label {
  font-size: 15px;
  font-weight: 600;
  color: var(--text-primary);
  margin-bottom: 16px;
}

.modal-grid-3 {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 12px;
}

.input-group {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.input-group label {
  font-size: 12px;
  font-weight: 600;
  color: var(--text-muted);
}

.input-group input,
.input-small {
  width: 100%;
  background: var(--bg-surface);
  border: 1px solid var(--border-default);
  color: var(--text-primary);
  font-family: inherit;
  font-size: 16px;
  font-weight: 600;
  padding: 10px 12px;
  border-radius: var(--radius-sm);
  outline: none;
  transition: all var(--transition-fast);
}

.input-small {
  width: 70px;
  text-align: center;
}

.input-group input:focus,
.input-small:focus {
  background: var(--bg-surface-active);
  border-color: rgba(255, 255, 255, 0.2);
}

.modal-row {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 12px 0;
}

.save-btn {
  width: 100%;
  background: var(--bg-surface-hover);
  color: var(--text-primary);
  border: 1px solid var(--border-strong);
  padding: 12px;
  font-family: inherit;
  font-size: 15px;
  font-weight: 600;
  border-radius: var(--radius-md);
  cursor: pointer;
  transition: all var(--transition-fast);
}

.save-btn:hover {
  background: var(--bg-surface-active);
  transform: translateY(-2px);
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
          if (this.onConnect) this.onConnect();
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
    this.state = 'off'; // off, focus, break, short-break, long-break, paused, done
    this.lastState = 'off';
    this.totalSeconds = 0;
    this.remainingSeconds = 0;
    this.interval = null;
    this.onTick = null;
    this.onDone = null;

    this.config = {
      focus: 25,
      shortBreak: 5,
      longBreak: 15,
      longBreakInterval: 4
    };

    const savedConfig = localStorage.getItem('trenzin_pomodoro_config');
    if (savedConfig) {
      try {
        this.config = { ...this.config, ...JSON.parse(savedConfig) };
      } catch (e) { }
    }

    this.completedPomodoros = 0;
  }

  start(type = 'focus') {
    this.stop();

    if (type === 'focus') {
      this.totalSeconds = this.config.focus * 60;
    } else if (type === 'long-break') {
      this.totalSeconds = this.config.longBreak * 60;
    } else { // 'break' or 'short-break'
      this.totalSeconds = this.config.shortBreak * 60;
    }

    this.remainingSeconds = this.totalSeconds;
    this.state = type;
    this.interval = setInterval(() => this._tick(), 1000);
    if (this.onTick) this.onTick();
  }

  pause() {
    if (this.state !== 'off' && this.state !== 'paused' && this.state !== 'done') {
      this.lastState = this.state;
      clearInterval(this.interval);
      this.interval = null;
      this.state = 'paused';
      if (this.onTick) this.onTick();
    }
  }

  resume() {
    if (this.state === 'paused') {
      this.state = this.lastState !== 'off' ? this.lastState : 'focus';
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
    if (this.state === 'off') {
      return `${String(this.config.focus).padStart(2, '0')}:00`;
    }
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

// ── SPA Router ─────────────────────────────────────────────
window.navigateTo = function (viewId) {
  $$('.view').forEach(view => {
    view.classList.remove('view--active');
  });
  const target = $('#view-' + viewId);
  if (target) {
    target.classList.add('view--active');
  }
};

function updateDateDisplay() {
  const date = new Date();
  const dateString = date.toLocaleDateString('pt-BR', { weekday: 'long', day: 'numeric', month: 'long' });
  const el = $('#date-display');
  if (el) el.textContent = 'Olá! ' + dateString.charAt(0).toUpperCase() + dateString.slice(1);
}

// ── Initialize ─────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
  setupEventListeners();
  updateTimerDisplay();
  initAlarm();
  updateLCDFace(currentExpression);
  updateLCDPreview();
  updateDateDisplay();
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
    trenzinConn.send(`TMR:FOCUS:${timer.config.focus}`);
    addLog(`TMR:FOCUS:${timer.config.focus}`, 'tx');
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
    timer.completedPomodoros++;
    if (timer.completedPomodoros % timer.config.longBreakInterval === 0) {
      timer.start('long-break');
      trenzinConn.send(`TMR:LBREAK:${timer.config.longBreak}`);
      addLog(`TMR:LBREAK:${timer.config.longBreak}`, 'tx');
    } else {
      timer.start('short-break');
      trenzinConn.send(`TMR:BREAK:${timer.config.shortBreak}`);
      addLog(`TMR:BREAK:${timer.config.shortBreak}`, 'tx');
    }
  });

  // Pomodoro Settings Modal
  $('#btn-pomodoro-settings').addEventListener('click', () => {
    $('#input-focus-time').value = timer.config.focus;
    $('#input-short-break').value = timer.config.shortBreak;
    $('#input-long-break').value = timer.config.longBreak;
    $('#input-long-interval').value = timer.config.longBreakInterval;
    $('#pomodoro-modal').classList.add('active');
  });

  $('#btn-close-modal').addEventListener('click', () => {
    $('#pomodoro-modal').classList.remove('active');
  });

  $('#btn-save-pomodoro').addEventListener('click', () => {
    timer.config.focus = parseInt($('#input-focus-time').value) || 25;
    timer.config.shortBreak = parseInt($('#input-short-break').value) || 5;
    timer.config.longBreak = parseInt($('#input-long-break').value) || 15;
    timer.config.longBreakInterval = parseInt($('#input-long-interval').value) || 4;

    localStorage.setItem('trenzin_pomodoro_config', JSON.stringify(timer.config));

    if (timer.state === 'off') {
      updateTimerDisplay();
    } else {
      updateTimerDisplay();
    }

    $('#pomodoro-modal').classList.remove('active');

    // update hardware
    trenzinConn.send(`TMR:FOCUS:${timer.config.focus}`);
  });

  // ── Alarm Logic ───────────────────────────────────────────
  $('#btn-alarm-add').addEventListener('click', () => {
    openAlarmModal(null); // null significa "novo alarme"
  });

  $('#btn-close-alarm-modal').addEventListener('click', () => {
    $('#alarm-modal').classList.remove('active');
  });

  $('#btn-save-alarm').addEventListener('click', () => {
    let h = parseInt($('#input-alarm-h').value) || 0;
    let m = parseInt($('#input-alarm-m').value) || 0;
    let label = $('#input-alarm-label').value.trim();
    if (h < 0) h = 0; if (h > 23) h = 23;
    if (m < 0) m = 0; if (m > 59) m = 59;

    if (editingAlarmId !== null) {
      // Edição
      const idx = alarms.findIndex(a => a.id === editingAlarmId);
      if (idx !== -1) {
        alarms[idx].h = h;
        alarms[idx].m = m;
        alarms[idx].label = label || `Alarme ${alarms[idx].id + 1}`;
        if (trenzinConn.connected) {
          trenzinConn.send(`ALM:SET:${editingAlarmId}:${h}:${m}`);
        }
      }
    } else {
      // Novo alarme
      if (alarms.length >= 10) {
        alert("Limite máximo de alarmes atingido.");
        return;
      }
      const newId = alarms.length > 0 ? Math.max(...alarms.map(a => a.id)) + 1 : 0;
      const newAlarm = { id: newId, h, m, enabled: true, label: label || `Alarme ${newId + 1}` };
      alarms.push(newAlarm);
      if (trenzinConn.connected) {
        trenzinConn.send(`ALM:SET:${newAlarm.id}:${newAlarm.h}:${newAlarm.m}`);
        trenzinConn.send(`ALM:ON:${newAlarm.id}`);
      }
    }

    saveAlarms();
    renderAlarms();
    $('#alarm-modal').classList.remove('active');
  });

  $('#btn-delete-alarm').addEventListener('click', () => {
    if (editingAlarmId !== null) {
      alarms = alarms.filter(a => a.id !== editingAlarmId);
      if (trenzinConn.connected) {
        trenzinConn.send(`ALM:DEL:${editingAlarmId}`);
      }
      saveAlarms();
      renderAlarms();
    }
    $('#alarm-modal').classList.remove('active');
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
        body: timer.totalSeconds > 5 * 60 ? 'Sessão de foco concluída!' : 'A pausa acabou!',
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
    addLog('Desconectado', 'system');
    return;
  }

  let ipToConnect;
  if (window.location.hostname && window.location.hostname !== 'localhost' && window.location.hostname !== '') {
    ipToConnect = window.location.hostname;
  } else {
    // Para desenvolvimento local (mock / debugging) sem o ESP32 real
    ipToConnect = '192.168.4.1'; // IP fallback padrão do Access Point do ESP32
  }

  trenzinConn.onConnect = () => {
    updateConnectionUI(true);
    addLog('Conectado ao Trenzin', 'system');

    alarms.forEach(a => {
      trenzinConn.send(`ALM:SET:${a.id}:${a.h}:${a.m}`);
      trenzinConn.send(a.enabled ? `ALM:ON:${a.id}` : `ALM:OFF:${a.id}`);
    });
  };

  trenzinConn.onReceive = (data) => {
    addLog(data, 'rx');
    parseArduinoMessage(data);
  };

  trenzinConn.onDisconnect = () => {
    updateConnectionUI(false);
    addLog('Trenzin desconectado', 'error');
  };

  addLog('Conectando ao WebSocket...', 'system');
  const success = await trenzinConn.connect(ipToConnect);

  if (!success) {
    updateConnectionUI(false);
    addLog('Falha na conexão', 'error');
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
    text.textContent = 'Conectado';
  } else {
    btn.classList.remove('connect-btn--connected');
    text.textContent = 'Conectar Trenzin';
  }

  // Enable/disable controls
  const overlayElements = $$('.card, .home-shortcuts');
  overlayElements.forEach(el => {
    el.classList.add('disabled-overlay');
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
  timeEl.textContent = timer.timeString;
  if (timer.state === 'off') {
    stateEl.textContent = 'Pronto';
  } else if (timer.state === 'done') {
    stateEl.textContent = 'Concluído!';
  } else {
    if (timer.state === 'paused') {
      stateEl.textContent = 'Pausado';
    } else if (timer.state === 'focus') {
      stateEl.textContent = 'Foco';
    } else if (timer.state === 'short-break' || timer.state === 'long-break') {
      stateEl.textContent = 'Pausa';
    } else if (timer.state === 'break') {
      stateEl.textContent = 'Pausa';
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
  $('#btn-pause').innerHTML = isPaused ?
    '<i data-lucide="play" width="16" height="16"></i> Retomar' :
    '<i data-lucide="pause" width="16" height="16"></i> Pausar';
  if (window.lucide) window.lucide.createIcons();
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
    lcdRow1 = `Foco  ${timer.timeString}`;
  } else if (timer.state === 'break' || timer.state === 'short-break' || timer.state === 'long-break') {
    lcdRow1 = `Pausa  ${timer.timeString}`;
  } else if (timer.state === 'paused') {
    lcdRow1 = `Pausado  ${timer.timeString}`;
  } else if (timer.state === 'done') {
    lcdRow1 = 'Concluído! :D';
  }
}

function updateLCDPreview() {
  $('#lcd-row-0').textContent = lcdRow0.trim();
  $('#lcd-row-1').textContent = lcdRow1.trim();
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

  const time = new Date().toLocaleTimeString('pt-BR', { hour12: false });
  entry.textContent = `[${time}] ${prefix[type] || ''}${message}\n`;
  log.appendChild(entry);
  log.scrollTop = log.scrollHeight;

  // Keep only last 100 entries
  while (log.children.length > 100) {
    log.removeChild(log.firstChild);
  }
}

// ── Alarm State & UI ──────────────────────────────────────
let alarms = [];
let editingAlarmId = null;

function initAlarm() {
  const saved = localStorage.getItem('trenzin_alarms');
  if (saved) {
    alarms = JSON.parse(saved);
  } else {
    alarms = [{ id: 0, h: 7, m: 0, enabled: false, label: "Alarme 1" }];
    saveAlarms();
  }
  renderAlarms();
}

function saveAlarms() {
  localStorage.setItem('trenzin_alarms', JSON.stringify(alarms));
}

function openAlarmModal(id) {
  editingAlarmId = id;
  const modal = $('#alarm-modal');
  const title = document.getElementById('alarm-modal-title-text') || modal.querySelector('.modal-section-title span');

  if (title) title.textContent = id !== null ? 'EDITAR ALARME' : 'NOVO ALARME';

  if (id !== null) {
    const a = alarms.find(x => x.id === id);
    $('#input-alarm-h').value = a ? a.h : 0;
    $('#input-alarm-m').value = a ? a.m : 0;
    $('#input-alarm-label').value = a ? (a.label || '') : '';
    $('#btn-delete-alarm').style.display = 'block';
  } else {
    $('#input-alarm-h').value = 7;
    $('#input-alarm-m').value = 0;
    $('#input-alarm-label').value = '';
    $('#btn-delete-alarm').style.display = 'none';
  }

  modal.classList.add('active');
}

function renderAlarms() {
  const list = $('#alarms-list');
  if (!list) return;
  list.innerHTML = '';

  alarms.forEach(a => {
    const hh = a.h.toString().padStart(2, '0');
    const mm = a.m.toString().padStart(2, '0');
    const stateClass = a.enabled ? 'alarm-card--on' : 'alarm-card--off';

    const card = document.createElement('div');
    card.className = `alarm-card ${stateClass}`;
    card.innerHTML = `
      <div class="alarm-card-header" style="justify-content: flex-start;">
        <button class="alarm-toggle-btn" data-id="${a.id}">
          ${a.enabled ? 'ON' : 'OFF'}
        </button>
      </div>
      <div class="alarm-card-bottom" style="margin-top:auto">
        <div class="alarm-card-label">${a.label}</div>
        <div class="alarm-card-time">${hh}:${mm}</div>
      </div>
    `;

    // Toggle
    const toggle = card.querySelector('.alarm-toggle-btn');
    toggle.addEventListener('click', (e) => {
      e.stopPropagation();
      a.enabled = !a.enabled;
      saveAlarms();
      renderAlarms();
      if (trenzinConn.connected) {
        trenzinConn.send(a.enabled ? `ALM:ON:${a.id}` : `ALM:OFF:${a.id}`);
      }
    });

    // Edit
    card.addEventListener('click', () => {
      openAlarmModal(a.id);
    });

    list.appendChild(card);
  });
}
)=====";

#endif
