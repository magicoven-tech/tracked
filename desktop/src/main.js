import { AppRenderer } from './renderer.js';
import { HandTracker } from './handTracker.js';
import { CanvasExporter } from './exporter.js';

document.addEventListener('DOMContentLoaded', async () => {
  // DOM Elements
  const webcam = document.getElementById('webcam');
  const threeCanvas = document.getElementById('threeCanvas');
  const skeletonCanvas = document.getElementById('skeletonCanvas');
  const statusBadge = document.getElementById('statusBadge');
  const statusText = document.getElementById('statusText');
  const gestureHint = document.getElementById('gestureHint');
  const controlsSidebar = document.getElementById('controlsSidebar');

  // Export Sidebar DOM
  const exportSidebar = document.getElementById('exportSidebar');
  const btnCloseExportSidebar = document.getElementById('btnCloseExportSidebar');
  const exportFormatGif = document.getElementById('exportFormatGif');
  const exportFormatMp4 = document.getElementById('exportFormatMp4');
  const speedBtns = document.querySelectorAll('#speedControlGroup .segment-btn');
  const fpsBtns = document.querySelectorAll('#fpsControlGroup .segment-btn');
  const resBtns = document.querySelectorAll('#resControlGroup .segment-btn');
  const exportEstimativeDetails = document.getElementById('exportEstimativeDetails');
  const btnDownloadExport = document.getElementById('btnDownloadExport');
  const exportBtnText = document.getElementById('exportBtnText');
  const exportProgressBarWrap = document.getElementById('exportProgressBarWrap');
  const exportProgressBar = document.getElementById('exportProgressBar');

  // Controls DOM
  const btnToggleSidebar = document.getElementById('btnToggleSidebar');
  const btnRecordScreen = document.getElementById('btnRecordScreen');
  const recBtnText = document.getElementById('recBtnText');
  const btnFullscreen = document.getElementById('btnFullscreen');
  const btnPause = document.getElementById('btnPause');
  const btnPlay = document.getElementById('btnPlay');
  const btnReset = document.getElementById('btnReset');
  const btnMotionVeil = document.getElementById('btnMotionVeil');
  const btnLiquidRipple = document.getElementById('btnLiquidRipple');
  const btnThermalVision = document.getElementById('btnThermalVision');
  const btnTouchDesigner = document.getElementById('btnTouchDesigner');
  const btnAscii = document.getElementById('btnAscii');
  const btnGlitch = document.getElementById('btnGlitch');
  const btnCircle = document.getElementById('btnCircle');

  const skeletonColorInput = document.getElementById('skeletonColor');
  const skeletonColorHex = document.getElementById('skeletonColorHex');

  const fabricOpacity = document.getElementById('fabricOpacity');
  const fabricOpacityVal = document.getElementById('fabricOpacityVal');

  const refractionStrength = document.getElementById('refractionStrength');
  const refractionStrengthVal = document.getElementById('refractionStrengthVal');

  const iridescence = document.getElementById('iridescence');
  const iridescenceVal = document.getElementById('iridescenceVal');

  const chromaticDispersion = document.getElementById('chromaticDispersion');
  const chromaticDispersionVal = document.getElementById('chromaticDispersionVal');

  const waveRipple = document.getElementById('waveRipple');
  const waveRippleVal = document.getElementById('waveRippleVal');

  const skeletonLineWidth = document.getElementById('skeletonLineWidth');
  const skeletonLineWidthVal = document.getElementById('skeletonLineWidthVal');

  const presetCards = document.querySelectorAll('.preset-card');

  // Initialize Renderer
  const appRenderer = new AppRenderer(threeCanvas, skeletonCanvas, webcam);
  const canvasExporter = new CanvasExporter(threeCanvas, skeletonCanvas, appRenderer);

  // Initialize Hand Tracker
  const tracker = new HandTracker(webcam, (results) => {
    appRenderer.updateHandLandmarks(results);
    if (results && results.multiHandLandmarks && results.multiHandLandmarks.length > 0) {
      gestureHint.style.opacity = '0.3';
    } else {
      gestureHint.style.opacity = '1';
    }
  });

  // Render Loop
  const loop = () => {
    appRenderer.render();
    if (canvasExporter && canvasExporter.isLiveRecording) {
      canvasExporter.captureLiveFrame();
    }
    requestAnimationFrame(loop);
  };
  loop();

  // Try Starting Camera
  updateStatus('Inicializando Câmera...', false);
  const camSuccess = await tracker.startCamera();
  if (camSuccess) {
    updateStatus('Câmera Ativa (Webcam)', true);
  } else {
    updateStatus('Modo Simulador Demo', true);
  }

  function updateStatus(text, isLive) {
    statusText.textContent = text;
    if (isLive) {
      statusBadge.classList.add('live-badge');
    } else {
      statusBadge.classList.remove('live-badge');
    }
  }

  // --- UI Event Handlers ---
  btnToggleSidebar.addEventListener('click', () => {
    const willOpen = controlsSidebar.classList.contains('collapsed');
    controlsSidebar.classList.toggle('collapsed');
    if (willOpen) {
      exportSidebar.classList.add('collapsed');
    }
  });

  btnFullscreen.addEventListener('click', () => {
    if (!document.fullscreenElement) {
      document.documentElement.requestFullscreen().catch(err => {
        console.warn('Fullscreen error:', err);
      });
    } else {
      if (document.exitFullscreen) {
        document.exitFullscreen();
      }
    }
  });

  // Play / Pause
  btnPause.addEventListener('click', () => {
    appRenderer.isPlaying = false;
    tracker.setPaused(true);
    btnPause.classList.add('active');
    btnPlay.classList.remove('active');
  });

  btnPlay.addEventListener('click', () => {
    appRenderer.isPlaying = true;
    tracker.setPaused(false);
    btnPlay.classList.add('active');
    btnPause.classList.remove('active');
  });

  // Effect Shader Switcher
  const effectBtns = [btnMotionVeil, btnLiquidRipple, btnThermalVision, btnTouchDesigner, btnAscii, btnGlitch, btnCircle];
  
  function setActiveEffectBtn(activeBtn) {
    effectBtns.forEach(btn => btn.classList.remove('active'));
    if (activeBtn) activeBtn.classList.add('active');
  }

  btnMotionVeil.addEventListener('click', () => {
    appRenderer.setEffectType('motionVeil');
    setActiveEffectBtn(btnMotionVeil);
  });

  btnLiquidRipple.addEventListener('click', () => {
    appRenderer.setEffectType('liquidRipple');
    setActiveEffectBtn(btnLiquidRipple);
  });

  btnThermalVision.addEventListener('click', () => {
    appRenderer.setEffectType('thermalVision');
    setActiveEffectBtn(btnThermalVision);
  });

  btnTouchDesigner.addEventListener('click', () => {
    appRenderer.setEffectType('touchDesignerPortal');
    setActiveEffectBtn(btnTouchDesigner);
  });

  btnAscii.addEventListener('click', () => {
    appRenderer.setEffectType('ascii');
    setActiveEffectBtn(btnAscii);
  });

  btnGlitch.addEventListener('click', () => {
    appRenderer.setEffectType('glitch');
    setActiveEffectBtn(btnGlitch);
  });

  btnCircle.addEventListener('click', () => {
    appRenderer.setEffectType('circle');
    setActiveEffectBtn(btnCircle);
  });

  // Color Input
  function setSkeletonColor(hex) {
    skeletonColorInput.value = hex;
    skeletonColorHex.value = hex.toUpperCase();
    appRenderer.skeletonColor = hex;
  }

  skeletonColorInput.addEventListener('input', (e) => {
    setSkeletonColor(e.target.value);
  });

  skeletonColorHex.addEventListener('change', (e) => {
    let hex = e.target.value;
    if (!hex.startsWith('#')) hex = '#' + hex;
    setSkeletonColor(hex);
  });

  // Range Slider Handlers
  fabricOpacity.addEventListener('input', (e) => {
    const val = parseFloat(e.target.value);
    fabricOpacityVal.textContent = val.toFixed(2);
    appRenderer.material.uniforms.uOpacity.value = val;
  });

  refractionStrength.addEventListener('input', (e) => {
    const val = parseFloat(e.target.value);
    refractionStrengthVal.textContent = Math.round(val);
    appRenderer.material.uniforms.uRefractionStrength.value = val;
  });

  iridescence.addEventListener('input', (e) => {
    const val = parseFloat(e.target.value);
    iridescenceVal.textContent = val.toFixed(2);
    appRenderer.material.uniforms.uIridescence.value = val;
  });

  chromaticDispersion.addEventListener('input', (e) => {
    const val = parseFloat(e.target.value);
    chromaticDispersionVal.textContent = val.toFixed(3);
    appRenderer.material.uniforms.uChromaticDispersion.value = val;
  });

  waveRipple.addEventListener('input', (e) => {
    const val = parseFloat(e.target.value);
    waveRippleVal.textContent = val.toFixed(2);
    appRenderer.material.uniforms.uWaveRipple.value = val;
  });

  skeletonLineWidth.addEventListener('input', (e) => {
    const val = parseFloat(e.target.value);
    skeletonLineWidthVal.textContent = val.toFixed(1) + 'px';
    appRenderer.skeletonThickness = val;
  });

  // Presets Configuration
  const PRESETS = {
    brik: {
      color: '#00ffaa',
      opacity: 0.9,
      refraction: 76,
      iridescence: 0.85,
      dispersion: 0.05,
      ripple: 0.4,
      lineWidth: 2.5
    },
    liquidGlass: {
      color: '#80deea',
      opacity: 0.65,
      refraction: 120,
      iridescence: 0.3,
      dispersion: 0.08,
      ripple: 0.2,
      lineWidth: 2.0
    },
    neonPrism: {
      color: '#00f0ff',
      opacity: 0.95,
      refraction: 90,
      iridescence: 1.0,
      dispersion: 0.12,
      ripple: 0.6,
      lineWidth: 3.5
    },
    silkCloth: {
      color: '#ff9a9e',
      opacity: 0.85,
      refraction: 40,
      iridescence: 0.7,
      dispersion: 0.03,
      ripple: 0.8,
      lineWidth: 2.5
    }
  };

  function applyPreset(name) {
    const p = PRESETS[name];
    if (!p) return;

    setSkeletonColor(p.color);

    fabricOpacity.value = p.opacity;
    fabricOpacityVal.textContent = p.opacity.toFixed(2);
    appRenderer.material.uniforms.uOpacity.value = p.opacity;

    refractionStrength.value = p.refraction;
    refractionStrengthVal.textContent = p.refraction;
    appRenderer.material.uniforms.uRefractionStrength.value = p.refraction;

    iridescence.value = p.iridescence;
    iridescenceVal.textContent = p.iridescence.toFixed(2);
    appRenderer.material.uniforms.uIridescence.value = p.iridescence;

    chromaticDispersion.value = p.dispersion;
    chromaticDispersionVal.textContent = p.dispersion.toFixed(3);
    appRenderer.material.uniforms.uChromaticDispersion.value = p.dispersion;

    waveRipple.value = p.ripple;
    waveRippleVal.textContent = p.ripple.toFixed(2);
    appRenderer.material.uniforms.uWaveRipple.value = p.ripple;

    skeletonLineWidth.value = p.lineWidth;
    skeletonLineWidthVal.textContent = p.lineWidth.toFixed(1) + 'px';
    appRenderer.skeletonThickness = p.lineWidth;

    presetCards.forEach(card => {
      card.classList.toggle('active', card.dataset.preset === name);
    });
  }

  presetCards.forEach(card => {
    card.addEventListener('click', () => {
      applyPreset(card.dataset.preset);
    });
  });

  btnReset.addEventListener('click', () => {
    applyPreset('brik');
  });

  // --- Export Sidebar Logic ---
  let selectedFormat = 'gif';
  let selectedSpeed = '1';
  let selectedFps = '15';
  let selectedRes = 'auto';

  function updateExportEstimation() {
    const config = canvasExporter.getConfig(selectedFormat, selectedSpeed, selectedFps, selectedRes);
    exportEstimativeDetails.textContent = `${config.width}×${config.height}px · ${config.frameCount} quadros · ~${config.sizeMb} MB`;
    exportBtnText.textContent = `Baixar ${selectedFormat.toUpperCase()}`;
  }

  // --- Record Screen Logic ---
  let isRecording = false;
  let recInterval = null;
  let recSeconds = 0;

  async function toggleRecordScreen() {
    isRecording = !isRecording;

    if (isRecording) {
      recSeconds = 0;
      btnRecordScreen.classList.add('recording');
      recBtnText.textContent = 'Parar (00:00)';
      canvasExporter.startLiveRecording();

      controlsSidebar.classList.add('collapsed');
      exportSidebar.classList.add('collapsed');

      recInterval = setInterval(() => {
        recSeconds++;
        const mins = String(Math.floor(recSeconds / 60)).padStart(2, '0');
        const secs = String(recSeconds % 60).padStart(2, '0');
        recBtnText.textContent = `Parar (${mins}:${secs})`;
      }, 1000);
    } else {
      clearInterval(recInterval);
      recInterval = null;
      btnRecordScreen.classList.remove('recording');
      recBtnText.textContent = 'Gravar tela';

      await canvasExporter.stopLiveRecording();

      controlsSidebar.classList.add('collapsed');
      exportSidebar.classList.remove('collapsed');
      updateExportEstimation();
    }
  }

  btnRecordScreen.addEventListener('click', toggleRecordScreen);

  // Close Export Sidebar
  btnCloseExportSidebar.addEventListener('click', () => {
    exportSidebar.classList.add('collapsed');
  });

  // Format Switchers
  exportFormatGif.addEventListener('click', () => {
    selectedFormat = 'gif';
    exportFormatGif.classList.add('active');
    exportFormatMp4.classList.remove('active');
    updateExportEstimation();
  });

  exportFormatMp4.addEventListener('click', () => {
    selectedFormat = 'mp4';
    exportFormatMp4.classList.add('active');
    exportFormatGif.classList.remove('active');
    updateExportEstimation();
  });

  // Speed Control
  speedBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      speedBtns.forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      selectedSpeed = btn.dataset.speed;
      updateExportEstimation();
    });
  });

  // FPS Control
  fpsBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      fpsBtns.forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      selectedFps = btn.dataset.fps;
      updateExportEstimation();
    });
  });

  // Resolution Control
  resBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      resBtns.forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      selectedRes = btn.dataset.res;
      updateExportEstimation();
    });
  });

  // Download Action
  btnDownloadExport.addEventListener('click', async () => {
    btnDownloadExport.disabled = true;
    exportProgressBarWrap.style.display = 'block';
    exportProgressBar.style.width = '0%';

    const formatUpper = selectedFormat.toUpperCase();
    exportBtnText.textContent = `Gerando ${formatUpper}... 0%`;

    try {
      await canvasExporter.export(
        selectedFormat,
        selectedSpeed,
        selectedFps,
        selectedRes,
        (progress) => {
          const percent = Math.round(progress * 100);
          exportProgressBar.style.width = `${percent}%`;
          exportBtnText.textContent = `Gerando ${formatUpper}... ${percent}%`;
        }
      );
    } catch (err) {
      console.error('Export error:', err);
      alert('Erro ao exportar o arquivo. Tente novamente.');
    } finally {
      btnDownloadExport.disabled = false;
      exportBtnText.textContent = `Baixar ${formatUpper}`;
      setTimeout(() => {
        exportProgressBarWrap.style.display = 'none';
        exportProgressBar.style.width = '0%';
      }, 1000);
    }
  });

  updateExportEstimation();

  // --- Remote Control HUD & Command Handler ---
  const remoteToast = document.getElementById('remoteToast');
  const remoteToastText = document.getElementById('remoteToastText');
  let remoteToastTimer = null;

  function showRemoteToast(title, subtitle = '') {
    if (!remoteToast || !remoteToastText) return;
    
    if (subtitle) {
      remoteToastText.innerHTML = `${title} <span class="remote-toast-sub">${subtitle}</span>`;
    } else {
      remoteToastText.textContent = title;
    }
    
    remoteToast.style.display = 'flex';
    remoteToast.style.animation = 'none';
    remoteToast.offsetHeight; // force reflow
    remoteToast.style.animation = 'toastSlideDown 0.35s cubic-bezier(0.16, 1, 0.3, 1)';

    if (remoteToastTimer) clearTimeout(remoteToastTimer);
    remoteToastTimer = setTimeout(() => {
      remoteToast.style.display = 'none';
    }, 2200);
  }

  function handleRemoteCommand({ topic, payload }) {
    console.log(`[Remote Control Command] ${topic}:`, payload);

    if (topic === 'magictracked/cmd/effect') {
      const effectNames = {
        'motionVeil': 'Motion Veil',
        'liquidRipple': 'Liquid Ripple',
        'thermalVision': 'Thermal Vision',
        'touchDesignerPortal': 'TouchDesigner Portal',
        'ascii': 'ASCII Art',
        'glitch': 'Glitch Effect',
        'circle': 'Effect Circle'
      };
      const effectMap = {
        'motionVeil': btnMotionVeil,
        'liquidRipple': btnLiquidRipple,
        'thermalVision': btnThermalVision,
        'touchDesignerPortal': btnTouchDesigner,
        'ascii': btnAscii,
        'glitch': btnGlitch,
        'circle': btnCircle
      };
      const targetBtn = effectMap[payload];
      if (targetBtn) {
        targetBtn.click();
        showRemoteToast('🎮 Efeito:', effectNames[payload] || payload);
      }
    } else if (topic === 'magictracked/cmd/preset') {
      const presetNames = {
        'brik': 'Brik Original',
        'liquidGlass': 'Liquid Glass',
        'neonPrism': 'Neon Prism',
        'silkCloth': 'Rainbow Silk'
      };
      applyPreset(payload);
      showRemoteToast('🎮 Preset:', presetNames[payload] || payload);
    } else if (topic === 'magictracked/cmd/param') {
      try {
        const data = typeof payload === 'object' ? payload : JSON.parse(payload);
        const { param, value } = data;

        const paramLabels = {
          'uOpacity': 'Opacidade',
          'uRefractionStrength': 'Refração',
          'uIridescence': 'Iridescência',
          'uChromaticDispersion': 'Dispersão RGB',
          'uWaveRipple': 'Ondulação',
          'skeletonLineWidth': 'Esqueleto'
        };

        let formattedVal = value;
        if (param === 'uOpacity') {
          fabricOpacity.value = value;
          fabricOpacity.dispatchEvent(new Event('input'));
          formattedVal = Math.round(value * 100) + '%';
        } else if (param === 'uRefractionStrength') {
          refractionStrength.value = value;
          refractionStrength.dispatchEvent(new Event('input'));
          formattedVal = Math.round(value);
        } else if (param === 'uIridescence') {
          iridescence.value = value;
          iridescence.dispatchEvent(new Event('input'));
          formattedVal = Math.round(value * 100) + '%';
        } else if (param === 'uChromaticDispersion') {
          chromaticDispersion.value = value;
          chromaticDispersion.dispatchEvent(new Event('input'));
          formattedVal = Number(value).toFixed(3);
        } else if (param === 'uWaveRipple') {
          waveRipple.value = value;
          waveRipple.dispatchEvent(new Event('input'));
          formattedVal = Math.round(value * 100) + '%';
        } else if (param === 'skeletonLineWidth') {
          skeletonLineWidth.value = value;
          skeletonLineWidth.dispatchEvent(new Event('input'));
          formattedVal = Number(value).toFixed(1) + 'px';
        }

        showRemoteToast(`🎮 ${paramLabels[param] || param}:`, formattedVal);
      } catch (e) {
        console.warn('Erro ao processar parâmetro MQTT:', e);
      }
    } else if (topic === 'magictracked/cmd/action') {
      const actionLabels = {
        'play': '▶ Play',
        'pause': '⏸ Pause',
        'rec_toggle': '🔴 Record Toggle',
        'reset': '🔄 Reset',
        'fullscreen': '⛶ Fullscreen',
        'connect_ack': 'ESP32 Conectado!'
      };

      if (payload === 'play') {
        btnPlay.click();
      } else if (payload === 'pause') {
        btnPause.click();
      } else if (payload === 'rec_toggle') {
        toggleRecordScreen();
      } else if (payload === 'reset') {
        btnReset.click();
      } else if (payload === 'fullscreen') {
        btnFullscreen.click();
      }

      showRemoteToast('🎮 Ação:', actionLabels[payload] || payload);
    }

    // Publish status feedback back to ESP32 via MQTT if in Electron
    if (window.electronAPI && typeof window.electronAPI.publishStatus === 'function') {
      window.electronAPI.publishStatus('magictracked/status/app', { status: 'ack', topic, payload });
    }
  }

  // 1. Electron IPC Listener
  if (window.electronAPI && typeof window.electronAPI.onRemoteControl === 'function') {
    console.log('[Remote Control] Listener MQTT registrado no Electron');
    window.electronAPI.onRemoteControl((data) => handleRemoteCommand(data));
  }

  // 2. WebSocket Fallback (for Web Browsers at localhost:5173 or remote IP)
  try {
    const wsHost = window.location.hostname || 'localhost';
    const ws = new WebSocket(`ws://${wsHost}:1884`);
    ws.onopen = () => {
      console.log('[Remote Control] Conectado ao servidor WebSocket do Mac (porta 1884)!');
    };
    ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        handleRemoteCommand(data);
      } catch (e) {
        console.warn('Erro ao ler mensagem WebSocket:', e);
      }
    };
  } catch (err) {
    console.log('[Remote Control] Conexão WebSocket não iniciada:', err);
  }
});

