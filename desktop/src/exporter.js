import { GIFEncoder, quantize, applyPalette } from './gifenc.js';
import { Muxer, ArrayBufferTarget } from './mp4Muxer.js';

export class CanvasExporter {
  constructor(threeCanvas, skeletonCanvas, appRenderer) {
    this.threeCanvas = threeCanvas;
    this.skeletonCanvas = skeletonCanvas;
    this.appRenderer = appRenderer;

    this.isLiveRecording = false;
    this.compositeCanvas = null;
    this.compositeCtx = null;

    this.frameSampleCanvas = null;
    this.frameSampleCtx = null;

    this.mediaRecorder = null;
    this.recordedChunks = [];
    this.recordedVideoBlob = null;
    this.recordedMimeType = '';
    this.recordedFrames = []; // Downscaled RGBA ImageData frames for GIF
    this.recordedDurationSec = 0;
    this.recordStartTime = 0;
    this.lastFrameTime = 0;
  }

  // Pick best supported video MIME type (prioritizing MP4/H.264 for QuickTime compatibility)
  getBestMimeType() {
    if (typeof MediaRecorder === 'undefined') return '';
    const mimeTypes = [
      'video/mp4;codecs=avc1.42E01E,mp4a.40.2',
      'video/mp4;codecs=avc1',
      'video/mp4;codecs=h264',
      'video/mp4',
      'video/webm;codecs=vp9',
      'video/webm;codecs=vp8',
      'video/webm'
    ];
    return mimeTypes.find(m => MediaRecorder.isTypeSupported(m)) || '';
  }

  // Draw combined WebGL scene + 2D skeleton onto composite canvas & capture frames for GIF
  captureLiveFrame() {
    if (!this.isLiveRecording || !this.compositeCtx) return;
    const w = this.compositeCanvas.width;
    const h = this.compositeCanvas.height;

    // Fill background with solid black first to avoid transparent/black pixel bugs
    this.compositeCtx.fillStyle = '#000000';
    this.compositeCtx.fillRect(0, 0, w, h);
    this.compositeCtx.drawImage(this.threeCanvas, 0, 0, w, h);
    this.compositeCtx.drawImage(this.skeletonCanvas, 0, 0, w, h);

    // Throttle frame sampling for GIF buffer (approx 15-20 FPS)
    const now = typeof performance !== 'undefined' ? performance.now() : Date.now();
    if (now - this.lastFrameTime >= 50) {
      this.lastFrameTime = now;

      // Downscale frame to 480px width max for lightweight RAM usage
      const sampleW = 480;
      const sampleH = Math.round((h / w) * sampleW);

      if (!this.frameSampleCanvas) {
        this.frameSampleCanvas = document.createElement('canvas');
      }
      if (this.frameSampleCanvas.width !== sampleW || this.frameSampleCanvas.height !== sampleH) {
        this.frameSampleCanvas.width = sampleW;
        this.frameSampleCanvas.height = sampleH;
        this.frameSampleCtx = this.frameSampleCanvas.getContext('2d', { willReadFrequently: true });
      }

      this.frameSampleCtx.fillStyle = '#000000';
      this.frameSampleCtx.fillRect(0, 0, sampleW, sampleH);
      this.frameSampleCtx.drawImage(this.compositeCanvas, 0, 0, sampleW, sampleH);

      const frameData = this.frameSampleCtx.getImageData(0, 0, sampleW, sampleH);
      // Ensure alpha is 255 for all pixels so palette quantization doesn't default to black
      for (let i = 3; i < frameData.data.length; i += 4) {
        frameData.data[i] = 255;
      }
      this.recordedFrames.push(frameData);
    }
  }

  startLiveRecording() {
    this.recordedChunks = [];
    this.recordedFrames = [];
    this.recordedVideoBlob = null;
    this.isLiveRecording = true;
    this.recordStartTime = typeof performance !== 'undefined' ? performance.now() : Date.now();
    this.lastFrameTime = 0;

    if (typeof document !== 'undefined') {
      const w = this.threeCanvas.width || 1280;
      const h = this.threeCanvas.height || 720;

      if (!this.compositeCanvas) {
        this.compositeCanvas = document.createElement('canvas');
      }
      this.compositeCanvas.width = w;
      this.compositeCanvas.height = h;
      this.compositeCtx = this.compositeCanvas.getContext('2d');

      this.captureLiveFrame();
    }

    if (typeof MediaRecorder !== 'undefined' && this.compositeCanvas) {
      this.recordedMimeType = this.getBestMimeType();
      const stream = this.compositeCanvas.captureStream(60);

      try {
        const options = this.recordedMimeType ? { mimeType: this.recordedMimeType, videoBitsPerSecond: 4000000 } : {};
        this.mediaRecorder = new MediaRecorder(stream, options);

        this.mediaRecorder.ondataavailable = (e) => {
          if (e.data && e.data.size > 0) {
            this.recordedChunks.push(e.data);
          }
        };

        this.mediaRecorder.start(100);
      } catch (err) {
        console.warn('MediaRecorder init error:', err);
      }
    }
  }

  stopLiveRecording() {
    this.isLiveRecording = false;
    this.recordedDurationSec = Math.max(0.5, ((typeof performance !== 'undefined' ? performance.now() : Date.now()) - this.recordStartTime) / 1000);

    return new Promise((resolve) => {
      if (this.mediaRecorder && this.mediaRecorder.state !== 'inactive') {
        this.mediaRecorder.onstop = () => {
          const mimeType = this.mediaRecorder.mimeType || this.recordedMimeType || 'video/webm';
          this.recordedVideoBlob = new Blob(this.recordedChunks, { type: mimeType });
          resolve({
            durationSec: this.recordedDurationSec,
            blob: this.recordedVideoBlob,
            mimeType
          });
        };
        this.mediaRecorder.stop();
      } else {
        resolve({
          durationSec: this.recordedDurationSec,
          blob: null,
          mimeType: ''
        });
      }
    });
  }

  // Get target export dimensions and estimate
  getConfig(format, speedVal, fpsVal, resOption) {
    const speed = parseFloat(speedVal) || 1.0;
    const fps = parseInt(fpsVal, 10) || 15;

    let srcW = this.threeCanvas.width || window.innerWidth;
    let srcH = this.threeCanvas.height || window.innerHeight;
    if (srcW === 0 || srcH === 0) {
      srcW = 1280;
      srcH = 720;
    }

    let width = srcW;
    let height = srcH;

    if (resOption === '50') {
      width = Math.round(srcW * 0.5);
      height = Math.round(srcH * 0.5);
    } else if (resOption === 'auto') {
      const maxDim = format === 'gif' ? 480 : 1080;
      if (srcW > maxDim || srcH > maxDim) {
        const scale = maxDim / Math.max(srcW, srcH);
        width = Math.round(srcW * scale);
        height = Math.round(srcH * scale);
      }
    }

    width = Math.max(2, width % 2 === 0 ? width : width - 1);
    height = Math.max(2, height % 2 === 0 ? height : height - 1);

    const baseDuration = this.recordedDurationSec > 0 ? this.recordedDurationSec : 3.5;
    const durationSec = baseDuration / speed;
    const frameCount = Math.max(10, Math.round(durationSec * fps));

    let estBytes = 0;
    if (format === 'gif') {
      estBytes = width * height * frameCount * 0.12;
    } else {
      estBytes = (1.5 * 1024 * 1024 * durationSec) / 8;
    }

    const sizeMb = (estBytes / (1024 * 1024)).toFixed(1);

    return {
      width,
      height,
      frameCount,
      durationSec,
      fps,
      speed,
      sizeMb: sizeMb === '0.0' ? '0.1' : sizeMb
    };
  }

  async export(format, speedVal, fpsVal, resOption, onProgress) {
    const config = this.getConfig(format, speedVal, fpsVal, resOption);
    const exportCanvas = document.createElement('canvas');
    exportCanvas.width = config.width;
    exportCanvas.height = config.height;
    const ctx = exportCanvas.getContext('2d', { willReadFrequently: true });

    const wasPlaying = this.appRenderer.isPlaying;

    if (format === 'gif') {
      await this.exportGif(exportCanvas, ctx, config, onProgress);
    } else {
      await this.exportMp4(exportCanvas, ctx, config, onProgress);
    }

    this.appRenderer.isPlaying = wasPlaying;
  }

  async exportGif(exportCanvas, ctx, config, onProgress) {
    const delayMs = Math.round(1000 / config.fps);
    const gif = GIFEncoder();

    const hasLiveFrames = this.recordedFrames.length > 0;
    const totalInput = hasLiveFrames ? this.recordedFrames.length : config.frameCount;
    const step = totalInput / config.frameCount;

    for (let f = 0; f < config.frameCount; f++) {
      ctx.fillStyle = '#000000';
      ctx.fillRect(0, 0, config.width, config.height);

      if (hasLiveFrames) {
        const frameIdx = Math.min(totalInput - 1, Math.floor(f * step));
        const srcFrame = this.recordedFrames[frameIdx];

        const tempCanvas = document.createElement('canvas');
        tempCanvas.width = srcFrame.width;
        tempCanvas.height = srcFrame.height;
        const tempCtx = tempCanvas.getContext('2d');

        tempCtx.putImageData(srcFrame, 0, 0);

        ctx.drawImage(tempCanvas, 0, 0, config.width, config.height);
      } else {
        const frameStepTime = (1.0 / config.fps) * config.speed;
        this.appRenderer.isPlaying = true;
        this.appRenderer.render();
        if (this.appRenderer.material && this.appRenderer.material.uniforms.uTime) {
          this.appRenderer.material.uniforms.uTime.value += frameStepTime;
        }

        ctx.drawImage(this.threeCanvas, 0, 0, config.width, config.height);
        ctx.drawImage(this.skeletonCanvas, 0, 0, config.width, config.height);
      }

      const imgData = ctx.getImageData(0, 0, config.width, config.height);
      // Ensure alpha is 255 for all pixels
      for (let i = 3; i < imgData.data.length; i += 4) {
        imgData.data[i] = 255;
      }

      const palette = quantize(imgData.data, 256);
      const index = applyPalette(imgData.data, palette);

      gif.writeFrame(index, config.width, config.height, {
        palette,
        delay: delayMs
      });

      if (onProgress) {
        onProgress((f + 1) / config.frameCount);
      }

      await new Promise(r => setTimeout(r, 5));
    }

    gif.finish();
    const buffer = gif.bytesView();
    const blob = new Blob([buffer], { type: 'image/gif' });

    this.downloadBlob(blob, `magic-tracked-${Date.now()}.gif`);
  }

  async exportMp4(exportCanvas, ctx, config, onProgress) {
    // Check if WebCodecs VideoEncoder + mp4-muxer is supported
    if (typeof VideoEncoder !== 'undefined' && typeof Muxer !== 'undefined') {
      try {
        const muxer = new Muxer({
          target: new ArrayBufferTarget(),
          video: {
            codec: 'avc',
            width: config.width,
            height: config.height
          },
          fastStart: 'in-memory'
        });

        const videoEncoder = new VideoEncoder({
          output: (chunk, meta) => muxer.addVideoChunk(chunk, meta),
          error: (e) => console.error('VideoEncoder error:', e)
        });

        videoEncoder.configure({
          codec: 'avc1.42001f', // Baseline H.264 profile for maximum QuickTime / macOS compatibility
          width: config.width,
          height: config.height,
          bitrate: 3_000_000,
          framerate: config.fps
        });

        const hasLiveFrames = this.recordedFrames.length > 0;
        const totalInput = hasLiveFrames ? this.recordedFrames.length : config.frameCount;
        const step = totalInput / config.frameCount;
        const frameMicroseconds = Math.round(1_000_000 / config.fps);

        for (let f = 0; f < config.frameCount; f++) {
          ctx.fillStyle = '#000000';
          ctx.fillRect(0, 0, config.width, config.height);

          if (hasLiveFrames) {
            const frameIdx = Math.min(totalInput - 1, Math.floor(f * step));
            const srcFrame = this.recordedFrames[frameIdx];

            const tempCanvas = document.createElement('canvas');
            tempCanvas.width = srcFrame.width;
            tempCanvas.height = srcFrame.height;
            const tempCtx = tempCanvas.getContext('2d');

            tempCtx.putImageData(srcFrame, 0, 0);
            ctx.drawImage(tempCanvas, 0, 0, config.width, config.height);
          } else {
            const frameStepTime = (1.0 / config.fps) * config.speed;
            this.appRenderer.isPlaying = true;
            this.appRenderer.render();
            if (this.appRenderer.material && this.appRenderer.material.uniforms.uTime) {
              this.appRenderer.material.uniforms.uTime.value += frameStepTime;
            }

            ctx.drawImage(this.threeCanvas, 0, 0, config.width, config.height);
            ctx.drawImage(this.skeletonCanvas, 0, 0, config.width, config.height);
          }

          const videoFrame = new VideoFrame(exportCanvas, {
            timestamp: f * frameMicroseconds
          });

          const isKeyFrame = f % (config.fps * 2) === 0;
          videoEncoder.encode(videoFrame, { keyFrame: isKeyFrame });
          videoFrame.close();

          if (onProgress) {
            onProgress((f + 1) / config.frameCount);
          }

          await new Promise(r => setTimeout(r, 5));
        }

        await videoEncoder.flush();
        muxer.finalize();

        const blob = new Blob([muxer.target.buffer], { type: 'video/mp4' });
        this.downloadBlob(blob, `magic-tracked-${Date.now()}.mp4`);
        return;
      } catch (err) {
        console.warn('WebCodecs / mp4-muxer encoding error, falling back:', err);
      }
    }

    // Fallback if WebCodecs is unavailable
    if (this.recordedVideoBlob) {
      const mime = this.recordedVideoBlob.type || '';
      const isWebm = mime.includes('webm');
      const ext = isWebm ? '.webm' : '.mp4';
      this.downloadBlob(this.recordedVideoBlob, `magic-tracked-${Date.now()}${ext}`);
      if (onProgress) onProgress(1.0);
      return;
    }
  }

  downloadBlob(blob, filename) {
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.style.display = 'none';
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    setTimeout(() => {
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
    }, 1000);
  }
}
