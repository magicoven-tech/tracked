import { describe, it } from 'node:test';
import assert from 'node:assert';
import { CanvasExporter } from '../src/exporter.js';

describe('Integration Tests - CanvasExporter', () => {
  const dummyThreeCanvas = { width: 1280, height: 720 };
  const dummySkeletonCanvas = { width: 1280, height: 720 };
  const dummyRenderer = { isPlaying: true, render: () => {} };

  it('should compute correct configuration for 100% resolution GIF', () => {
    const exporter = new CanvasExporter(dummyThreeCanvas, dummySkeletonCanvas, dummyRenderer);
    const config = exporter.getConfig('gif', '1', '15', '100');

    assert.strictEqual(config.width, 1280);
    assert.strictEqual(config.height, 720);
    assert.strictEqual(config.fps, 15);
    assert.strictEqual(config.frameCount, 53); // 3.5s * 15fps
    assert.ok(parseFloat(config.sizeMb) > 0);
  });

  it('should scale dimensions properly for 50% resolution', () => {
    const exporter = new CanvasExporter(dummyThreeCanvas, dummySkeletonCanvas, dummyRenderer);
    const config = exporter.getConfig('mp4', '1', '24', '50');

    assert.strictEqual(config.width, 640);
    assert.strictEqual(config.height, 360);
    assert.strictEqual(config.fps, 24);
  });

  it('should manage live recording lifecycle correctly', async () => {
    const exporter = new CanvasExporter(dummyThreeCanvas, dummySkeletonCanvas, dummyRenderer);
    assert.strictEqual(exporter.isLiveRecording, false);

    exporter.startLiveRecording();
    assert.strictEqual(exporter.isLiveRecording, true);

    const stats = await exporter.stopLiveRecording();
    assert.strictEqual(exporter.isLiveRecording, false);
    assert.ok(stats.durationSec >= 0);
  });
});
