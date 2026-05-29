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
    $('#pomodoro-modal').classList.remove('active');

    // Update config
    timer.config.focus = parseInt($('#input-focus-time').value) || 25;
    timer.config.shortBreak = parseInt($('#input-short-break').value) || 5;
    timer.config.longBreak = parseInt($('#input-long-break').value) || 15;
    timer.config.longBreakInterval = parseInt($('#input-long-interval').value) || 4;

    localStorage.setItem('trenzin_pomodoro_config', JSON.stringify(timer.config));

    if (timer.state === 'off') {
      updateTimerDisplay(); // Refresh the default '25:00' to whatever is set
    }
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
  if (window.location.hostname && window.location.hostname !== 'localhost') {
    ipToConnect = window.location.hostname;
  } else {
    addLog('Não foi possível determinar o endereço IP', 'error');
    return;
  }

  // Set up handlers before connecting so early messages aren't lost
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

  if (success) {
    updateConnectionUI(true);
    addLog('Conectado ao Trenzin', 'system');

    trenzinConn.onReceive = (data) => {
      addLog(data, 'rx');
      parseArduinoMessage(data);
    };

    trenzinConn.onDisconnect = () => {
      updateConnectionUI(false);
      addLog('Trenzin desconectado', 'error');
    };
  } else {
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
