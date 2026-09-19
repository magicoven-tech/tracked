/**
 * Fast, lightweight zero-dependency GIF89a Encoder for HTML5 Canvas
 */
export class GifEncoder {
  constructor(width, height, delayMs = 66) {
    this.width = width;
    this.height = height;
    this.delayMs = delayMs;
    this.frames = [];
    this.data = [];
  }

  addFrame(ctx, delayMs = this.delayMs) {
    const imgData = ctx.getImageData(0, 0, this.width, this.height);
    this.frames.push({ imgData, delay: Math.round(delayMs / 10) });
  }

  // Quantize RGBA to 256-color palette (Simple & fast uniform/median cut quantization)
  quantize(rgba) {
    const palette = [];
    const colorMap = new Map();
    const indexedPixels = new Uint8Array(rgba.length / 4);

    let nextIndex = 0;

    for (let i = 0; i < rgba.length; i += 4) {
      const r = rgba[i];
      const g = rgba[i + 1];
      const b = rgba[i + 2];
      
      // Quantize 8-bit to 5-bit per channel for color map lookup (32x32x32 = 32768 colors max)
      const qr = r & 0xf8;
      const qg = g & 0xf8;
      const qb = b & 0xf8;
      const key = (qr << 10) | (qg << 5) | qb;

      let idx = colorMap.get(key);
      if (idx === undefined) {
        if (palette.length < 256) {
          idx = palette.length;
          palette.push([qr, qg, qb]);
          colorMap.set(key, idx);
        } else {
          // Find nearest existing palette color
          let minDist = Infinity;
          idx = 0;
          for (let p = 0; p < palette.length; p++) {
            const dr = r - palette[p][0];
            const dg = g - palette[p][1];
            const db = b - palette[p][2];
            const dist = dr * dr + dg * dg + db * db;
            if (dist < minDist) {
              minDist = dist;
              idx = p;
            }
          }
        }
      }
      indexedPixels[i / 4] = idx;
    }

    // Pad palette to 256 entries
    while (palette.length < 256) {
      palette.push([0, 0, 0]);
    }

    return { palette, indexedPixels };
  }

  // LZW Encoder for GIF
  lzwEncode(width, height, indexedPixels, minCodeSize) {
    const out = [];
    let accum = 0;
    let bits = 0;

    const writeBits = (val, count) => {
      accum |= val << bits;
      bits += count;
      while (bits >= 8) {
        out.push(accum & 0xff);
        accum >>= 8;
        bits -= 8;
      }
    };

    const flushBits = () => {
      if (bits > 0) {
        out.push(accum & 0xff);
        accum = 0;
        bits = 0;
      }
    };

    const clearCode = 1 << minCodeSize;
    const eofCode = clearCode + 1;
    let codeSize = minCodeSize + 1;
    let nextCode = eofCode + 1;

    let trie = new Map();

    const resetTrie = () => {
      trie.clear();
      codeSize = minCodeSize + 1;
      nextCode = eofCode + 1;
    };

    writeBits(clearCode, codeSize);
    resetTrie();

    let ent = indexedPixels[0];

    for (let i = 1; i < indexedPixels.length; i++) {
      const c = indexedPixels[i];
      const key = (ent << 16) | c;
      if (trie.has(key)) {
        ent = trie.get(key);
      } else {
        writeBits(ent, codeSize);
        if (nextCode < 4096) {
          trie.set(key, nextCode++);
          if (nextCode === (1 << codeSize) && codeSize < 12) {
            codeSize++;
          }
        } else {
          writeBits(clearCode, codeSize);
          resetTrie();
        }
        ent = c;
      }
    }

    writeBits(ent, codeSize);
    writeBits(eofCode, codeSize);
    flushBits();

    // Package into GIF sub-blocks (max 255 bytes per block)
    const blocks = [];
    for (let i = 0; i < out.length; i += 255) {
      const chunk = out.slice(i, i + 255);
      blocks.push(chunk.length);
      blocks.push(...chunk);
    }
    blocks.push(0); // Block terminator

    return blocks;
  }

  // Write Uint16 / Uint8 helpers
  writeString(arr, str) {
    for (let i = 0; i < str.length; i++) {
      arr.push(str.charCodeAt(i));
    }
  }

  writeUint16(arr, val) {
    arr.push(val & 0xff, (val >> 8) & 0xff);
  }

  render(onProgress) {
    const bytes = [];

    // 1. Header: GIF89a
    this.writeString(bytes, 'GIF89a');

    // 2. Logical Screen Descriptor
    this.writeUint16(bytes, this.width);
    this.writeUint16(bytes, this.height);
    // Global Color Table Flag (0), Color Resolution (7 = 8bits), Sort (0), GCT Size (0)
    bytes.push(0x70, 0x00, 0x00);

    // 3. Application Extension for Infinite Looping (Netscape 2.0)
    bytes.push(0x21, 0xff, 0x0b);
    this.writeString(bytes, 'NETSCAPE2.0');
    bytes.push(0x03, 0x01, 0x00, 0x00, 0x00); // Loop count = 0 (forever)

    // 4. Encode each frame
    const totalFrames = this.frames.length;

    for (let f = 0; f < totalFrames; f++) {
      const { imgData, delay } = this.frames[f];
      const { palette, indexedPixels } = this.quantize(imgData.data);

      // Graphic Control Extension
      bytes.push(0x21, 0xf9, 0x04);
      bytes.push(0x04); // Disposal method 1 (do not dispose), user input 0, transparent color 0
      this.writeUint16(bytes, delay); // Delay time (in 1/100ths of a second)
      bytes.push(0x00); // Transparent color index
      bytes.push(0x00); // Block terminator

      // Image Descriptor
      bytes.push(0x2c);
      this.writeUint16(bytes, 0); // Image Left
      this.writeUint16(bytes, 0); // Image Top
      this.writeUint16(bytes, this.width);
      this.writeUint16(bytes, this.height);
      // Local Color Table Flag (1), Interlace (0), Sort (0), LCT Size (7 = 256 colors)
      bytes.push(0x87);

      // Local Color Table (256 RGB entries = 768 bytes)
      for (let i = 0; i < 256; i++) {
        const [r, g, b] = palette[i];
        bytes.push(r, g, b);
      }

      // LZW Minimum Code Size
      const minCodeSize = 8;
      bytes.push(minCodeSize);

      // LZW Image Data
      const lzwData = this.lzwEncode(this.width, this.height, indexedPixels, minCodeSize);
      bytes.push(...lzwData);

      if (onProgress) {
        onProgress((f + 1) / totalFrames);
      }
    }

    // 5. GIF Trailer
    bytes.push(0x3b);

    return new Blob([new Uint8Array(bytes)], { type: 'image/gif' });
  }
}
