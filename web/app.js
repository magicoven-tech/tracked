// ============================================================
// TABY Clone — Web Serial Dashboard
// Communicates with Arduino Nano via Web Serial API
// ============================================================

// ── Serial Connection Manager ──────────────────────────────
class SerialConnection {
  constructor() {
    this.port = null;
    this.reader = null;
    this.writer = null;
    this.readableStreamClosed = null;
    this.writableStreamClosed = null;
    this.connected = false;
    this.onReceive = null;
    this.onDisconnect = null;
    this._buffer = '';
  }

  async connect() {
    try {
      this.port = await navigator.serial.requestPort();
      await this.port.open({ baudRate: 9600 });
      this.connected = true;

      // Start reading
      this._startReading();

      // Listen for disconnect
      navigator.serial.addEventListener('disconnect', (e) => {
        if (e.target === this.port) {
          this.connected = false;
          if (this.onDisconnect) this.onDisconnect();
        }
      });

      return true;
    } catch (err) {
      console.error('Connection failed:', err);
      return false;
    }
  }

  async _startReading() {
    const decoder = new TextDecoderStream();
    this.readableStreamClosed = this.port.readable.pipeTo(decoder.writable);
    const inputStream = decoder.readable;
    this.reader = inputStream.getReader();

    try {
      while (true) {
        const { value, done } = await this.reader.read();
        if (done) break;
        if (value) {
          this._buffer += value;
          const lines = this._buffer.split('\n');
          // Process all complete lines
          for (let i = 0; i < lines.length - 1; i++) {
            const line = lines[i].trim();
            if (line && this.onReceive) {
              this.onReceive(line);
            }
          }
          // Keep the last incomplete line in the buffer
          this._buffer = lines[lines.length - 1];
        }
      }
    } catch (err) {
      if (this.connected) {
        console.error('Read error:', err);
      }
    }
  }

  async send(data) {
    if (!this.connected || !this.port?.writable) return false;
    try {
      const writer = this.port.writable.getWriter();
      const encoder = new TextEncoder();
      await writer.write(encoder.encode(data + '\n'));
      writer.releaseLock();
      return true;
    } catch (err) {
      console.error('Send error:', err);
      return false;
    }
  }

  async disconnect() {
    this.connected = false;
    try {
      if (this.reader) {
        await this.reader.cancel();
        await this.readableStreamClosed.catch(() => {});
        this.reader = null;
      }
      if (this.port) {
        await this.port.close();
        this.port = null;
      }
    } catch (err) {
      console.error('Disconnect error:', err);
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
const serial = new SerialConnection();
const timer = new PomodoroTimer();
let currentExpression = 'idle';
let lcdRow0 = '                ';
let lcdRow1 = '                ';

// ── DOM References ─────────────────────────────────────────
const $ = (sel) => document.querySelector(sel);
const $$ = (sel) => document.querySelectorAll(sel);

// ── Initialize ─────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
  // Check for Web Serial support
  if (!('serial' in navigator)) {
    addLog('Web Serial API not supported. Use Chrome or Edge.', 'error');
    $('#connect-btn').disabled = true;
    $('#connect-btn').textContent = 'Not Supported';
    return;
  }

  setupEventListeners();
  updateTimerDisplay();
  updateLCDPreview();
  updateConnectionUI(false);
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
    serial.send('TMR:START');
    addLog('TMR:START', 'tx');
  });

  $('#btn-pause').addEventListener('click', () => {
    if (timer.state === 'paused') {
      timer.resume();
      serial.send('TMR:RESUME');
      addLog('TMR:RESUME', 'tx');
    } else {
      timer.pause();
      serial.send('TMR:PAUSE');
      addLog('TMR:PAUSE', 'tx');
    }
  });

  $('#btn-stop').addEventListener('click', () => {
    timer.stop();
    serial.send('TMR:STOP');
    addLog('TMR:STOP', 'tx');
  });

  $('#btn-break').addEventListener('click', () => {
    timer.start('break');
    serial.send('TMR:BREAK');
    addLog('TMR:BREAK', 'tx');
  });

  // Message form
  $('#message-form').addEventListener('submit', (e) => {
    e.preventDefault();
    const input = $('#message-input');
    const msg = input.value.trim();
    if (msg) {
      serial.send('MSG:' + msg);
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
      new Notification('Taby', {
        body: timer.totalSeconds > 5 * 60 ? '🎉 Focus session done!' : '☕ Break is over!',
        icon: 'data:image/svg+xml,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100"><text y="80" font-size="80">🤖</text></svg>'
      });
    }
  };
}

// ── Connection ─────────────────────────────────────────────
async function handleConnect() {
  if (serial.connected) {
    await serial.disconnect();
    updateConnectionUI(false);
    addLog('Disconnected', 'system');
    return;
  }

  // Set up handlers before connecting so early messages aren't lost
  serial.onReceive = (data) => {
    addLog(data, 'rx');
    parseArduinoMessage(data);
  };

  serial.onDisconnect = () => {
    updateConnectionUI(false);
    addLog('Arduino disconnected', 'error');
  };

  addLog('Requesting serial port...', 'system');
  const success = await serial.connect();

  if (success) {
    updateConnectionUI(true);
    addLog('Connected to Arduino', 'system');

    // Request notification permission
    if (Notification.permission === 'default') {
      Notification.requestPermission();
    }
  } else {
    addLog('Connection failed or cancelled', 'error');
  }
}

// ── Send Expression ────────────────────────────────────────
function sendExpression(expr) {
  currentExpression = expr;
  const cmd = 'EXP:' + expr.toUpperCase();
  serial.send(cmd);
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
    idle:  '     O    O     ',
    happy: '     ^ vv ^     ',
    sad:   '     T    T     ',
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
