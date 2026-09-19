import { describe, it } from 'node:test';
import assert from 'node:assert';
import { GifEncoder } from '../src/gifEncoder.js';

describe('Unit Tests - GifEncoder', () => {
  it('should initialize GifEncoder with specified width, height and delay', () => {
    const encoder = new GifEncoder(320, 240, 50);
    assert.strictEqual(encoder.width, 320);
    assert.strictEqual(encoder.height, 240);
    assert.strictEqual(encoder.delayMs, 50);
    assert.strictEqual(encoder.frames.length, 0);
  });

  it('should quantize RGBA buffer into 256 color palette', () => {
    const encoder = new GifEncoder(2, 2);
    // 2x2 red image
    const rgba = new Uint8ClampedArray([
      255, 0, 0, 255,   255, 0, 0, 255,
      255, 0, 0, 255,   255, 0, 0, 255
    ]);
    const { palette, indexedPixels } = encoder.quantize(rgba);

    assert.strictEqual(indexedPixels.length, 4);
    assert.strictEqual(palette.length, 256);
    assert.strictEqual(palette[0][0], 248); // quantized 255 & 0xf8
  });

  it('should generate valid LZW block stream', () => {
    const encoder = new GifEncoder(2, 2);
    const indexedPixels = new Uint8Array([0, 0, 0, 0]);
    const lzwBlocks = encoder.lzwEncode(2, 2, indexedPixels, 8);

    assert.ok(lzwBlocks.length > 0);
    assert.strictEqual(lzwBlocks[lzwBlocks.length - 1], 0); // Block terminator 0x00
  });
});
