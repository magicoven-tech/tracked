export class HandTracker {
  constructor(videoElement, onResultsCallback) {
    this.video = videoElement;
    this.onResultsCallback = onResultsCallback;
    this.camera = null;
    this.hands = null;
    this.isDemoMode = false;
    this.isCameraActive = false;
    this.isPaused = false;
    this.demoAnimFrame = null;
    this.demoTime = 0;

    this.init();
  }

  async init() {
    try {
      const HandsClass = window.Hands;

      this.hands = new HandsClass({
        locateFile: (file) => `https://cdn.jsdelivr.net/npm/@mediapipe/hands/${file}`
      });

      this.hands.setOptions({
        maxNumHands: 2,
        modelComplexity: 1,
        minDetectionConfidence: 0.5,
        minTrackingConfidence: 0.5
      });

      this.hands.onResults((results) => {
        if (!this.isPaused && this.onResultsCallback) {
          this.onResultsCallback(results);
        }
      });
    } catch (err) {
      console.warn('MediaPipe Hands initialization fallback to window:', err);
    }
  }

  setPaused(isPaused) {
    this.isPaused = isPaused;
    if (isPaused) {
      if (this.video && this.video.pause) this.video.pause();
    } else {
      if (this.video && this.video.play) this.video.play().catch(() => {});
    }
  }

  async startCamera() {
    this.stopDemo();
    try {
      const stream = await navigator.mediaDevices.getUserMedia({
        video: { width: { ideal: 1280 }, height: { ideal: 720 }, facingMode: 'user' },
        audio: false
      });

      this.video.srcObject = stream;
      await this.video.play();

      this.isCameraActive = true;
      this.isPaused = false;

      const CameraClass = window.Camera;

      this.camera = new CameraClass(this.video, {
        onFrame: async () => {
          if (this.isCameraActive && !this.isPaused && this.hands) {
            await this.hands.send({ image: this.video });
          }
        },
        width: 1280,
        height: 720
      });

      await this.camera.start();
      return true;
    } catch (err) {
      console.error('Camera access denied or error:', err);
      this.startDemoMode();
      return false;
    }
  }

  stopCamera() {
    if (this.camera) {
      this.camera.stop();
      this.camera = null;
    }
    if (this.video.srcObject) {
      const tracks = this.video.srcObject.getTracks();
      tracks.forEach(track => track.stop());
      this.video.srcObject = null;
    }
    this.isCameraActive = false;
  }

  startDemoMode() {
    this.stopCamera();
    this.isDemoMode = true;
    this.isPaused = false;

    // Simulated 2 hands movement loop for demo
    const loop = () => {
      if (!this.isDemoMode) return;

      if (!this.isPaused) {
        this.demoTime += 0.03;
        const t = this.demoTime;

        // Simulated Left Hand landmarks (21 points)
        const leftCenter = {
          x: 0.32 + Math.sin(t * 0.8) * 0.08,
          y: 0.5 + Math.cos(t * 1.2) * 0.06,
          z: 0
        };

        // Simulated Right Hand landmarks (21 points)
        const rightCenter = {
          x: 0.68 + Math.cos(t * 0.9) * 0.08,
          y: 0.5 + Math.sin(t * 1.1) * 0.06,
          z: 0
        };

        const generateHand = (center, isLeft) => {
          const landmarks = [];
          const dir = isLeft ? -1 : 1;

          // Wrist
          landmarks[0] = { x: center.x, y: center.y + 0.12, z: 0 };

          // Thumb, Index, Middle, Ring, Pinky
          const fingerAngles = [-0.5, -0.15, 0.1, 0.35, 0.6];
          let idx = 1;
          for (let f = 0; f < 5; f++) {
            const angle = fingerAngles[f] * dir;
            let px = center.x + Math.sin(angle) * 0.03 * dir;
            let py = center.y + Math.cos(angle) * 0.03;
            for (let j = 0; j < 4; j++) {
              px += Math.sin(angle + Math.sin(t + f) * 0.08) * 0.025 * dir;
              py -= Math.cos(angle) * 0.03;
              landmarks[idx++] = { x: px, y: py, z: (j * 0.01) };
            }
          }
          return landmarks;
        };

        const simulatedResults = {
          multiHandLandmarks: [
            generateHand(leftCenter, true),
            generateHand(rightCenter, false)
          ],
          multiHandedness: [
            { label: 'Left' },
            { label: 'Right' }
          ]
        };

        if (this.onResultsCallback) {
          this.onResultsCallback(simulatedResults);
        }
      }

      this.demoAnimFrame = requestAnimationFrame(loop);
    };

    loop();
  }

  stopDemo() {
    this.isDemoMode = false;
    if (this.demoAnimFrame) {
      cancelAnimationFrame(this.demoAnimFrame);
      this.demoAnimFrame = null;
    }
  }
}
