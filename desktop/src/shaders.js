export const IridescentFabricShader = {
  uniforms: {
    uTexture: { value: null },
    uRefractionStrength: { value: 0.76 },
    uOpacity: { value: 0.9 },
    uIridescence: { value: 0.85 },
    uChromaticDispersion: { value: 0.05 },
    uWaveRipple: { value: 0.4 },
    uTime: { value: 0.0 },
    uResolution: { value: [1.0, 1.0] }
  },

  vertexShader: /* glsl */ `
    uniform float uTime;
    uniform float uWaveRipple;

    varying vec2 vUv;
    varying vec2 vScreenUv;
    varying vec3 vNormal;
    varying vec3 vViewPosition;
    varying vec3 vWorldPosition;

    void main() {
      vUv = uv;

      // Calculate dynamic wave ripples across fabric surface
      vec3 pos = position;
      float wave1 = sin(pos.x * 6.0 + uTime * 3.0) * cos(pos.y * 6.0 + uTime * 2.5);
      float wave2 = cos(pos.x * 12.0 - uTime * 4.0) * sin(pos.y * 10.0 + uTime * 3.5);
      float totalWave = (wave1 * 0.7 + wave2 * 0.3) * uWaveRipple * 0.12;

      pos += normal * totalWave;

      vec4 worldPosition = modelMatrix * vec4(pos, 1.0);
      vWorldPosition = worldPosition.xyz;

      vec4 mvPosition = viewMatrix * worldPosition;
      vViewPosition = -mvPosition.xyz;

      vNormal = normalize(normalMatrix * normal);

      gl_Position = projectionMatrix * mvPosition;
      vScreenUv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
    }
  `,

  fragmentShader: /* glsl */ `
    uniform sampler2D uTexture;
    uniform float uRefractionStrength;
    uniform float uOpacity;
    uniform float uIridescence;
    uniform float uChromaticDispersion;
    uniform float uTime;

    varying vec2 vUv;
    varying vec2 vScreenUv;
    varying vec3 vNormal;
    varying vec3 vViewPosition;
    varying vec3 vWorldPosition;

    // Cosine color palette for smooth holographic rainbow gradients
    vec3 rainbowPalette(in float t) {
      vec3 a = vec3(0.5, 0.5, 0.5);
      vec3 b = vec3(0.5, 0.5, 0.5);
      vec3 c = vec3(1.0, 1.0, 1.0);
      vec3 d = vec3(0.0, 0.33, 0.67);
      return a + b * cos(6.28318 * (c * t + d));
    }

    void main() {
      vec3 normal = normalize(vNormal);
      vec3 viewDir = normalize(vViewPosition);

      // Use screen space UV for true refraction (aligns perfectly with background camera)
      vec2 baseUv = vScreenUv;

      // Refraction displacement offset computed from surface normal
      float offsetScale = uRefractionStrength * 0.08;
      vec2 distortion = normal.xy * offsetScale;

      // Chromatic dispersion (RGB channel split)
      float dispersion = uChromaticDispersion * 0.4;
      vec2 uvR = baseUv + distortion * (1.0 + dispersion);
      vec2 uvG = baseUv + distortion;
      vec2 uvB = baseUv + distortion * (1.0 - dispersion);

      // Sample webcam texture with RGB split
      float r = texture2D(uTexture, clamp(uvR, 0.001, 0.999)).r;
      float g = texture2D(uTexture, clamp(uvG, 0.001, 0.999)).g;
      float b = texture2D(uTexture, clamp(uvB, 0.001, 0.999)).b;
      vec3 refractedColor = vec3(r, g, b);

      // Fresnel effect for edge glow and iridescence angle dependence
      float fresnel = pow(1.0 - max(dot(viewDir, normal), 0.0), 2.5);

      // Rainbow holographic tint based on view angle and position
      float rainbowPhase = fresnel * 1.8 + vWorldPosition.x * 0.5 + vWorldPosition.y * 0.5 + uTime * 0.15;
      vec3 iridescentColor = rainbowPalette(rainbowPhase);

      // Specular highlight
      vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));
      vec3 halfDir = normalize(lightDir + viewDir);
      float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
      vec3 specularColor = vec3(1.0) * spec * 0.6;

      // Combine refracted video feed with iridescence and sheen
      vec3 finalColor = mix(refractedColor, iridescentColor, fresnel * uIridescence * 0.7);
      finalColor += specularColor + iridescentColor * uIridescence * 0.15;

      // Subtle edge sheen highlight
      finalColor += vec3(0.8, 1.0, 0.9) * pow(fresnel, 4.0) * 0.5;

      gl_FragColor = vec4(finalColor, uOpacity);
    }
  `
};

export const LiquidRippleShader = {
  uniforms: {
    uTexture: { value: null },
    uRefractionStrength: { value: 1.5 },
    uOpacity: { value: 0.95 },
    uWaveRipple: { value: 0.8 },
    uTime: { value: 0.0 },
    uResolution: { value: [1.0, 1.0] },
    // Reusing these uniforms so the sliders still work without errors
    uIridescence: { value: 0.2 },
    uChromaticDispersion: { value: 0.1 }
  },

  vertexShader: /* glsl */ `
    uniform float uTime;
    uniform float uWaveRipple;

    varying vec2 vUv;
    varying vec2 vScreenUv;
    varying vec3 vNormal;
    varying vec3 vViewPosition;
    varying vec3 vWorldPosition;

    void main() {
      vUv = uv;

      // Heavy radial ripples and displacement for liquid effect
      vec3 pos = position;
      
      // Calculate distance from center
      float dist = length(pos.xy);
      
      // Circular wave originating from the center
      float radialWave = sin(dist * 15.0 - uTime * 6.0) * exp(-dist * 1.5);
      
      // Add a secondary wave for chaos
      float noiseWave = sin(pos.x * 5.0 + uTime * 2.0) * cos(pos.y * 5.0 - uTime * 3.0);
      
      float totalWave = (radialWave * 0.8 + noiseWave * 0.2) * uWaveRipple * 0.4;
      
      // Extrude along the normal
      pos += normal * totalWave;

      vec4 worldPosition = modelMatrix * vec4(pos, 1.0);
      vWorldPosition = worldPosition.xyz;

      vec4 mvPosition = viewMatrix * worldPosition;
      vViewPosition = -mvPosition.xyz;

      vNormal = normalize(normalMatrix * normal);

      gl_Position = projectionMatrix * mvPosition;
      vScreenUv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
    }
  `,

  fragmentShader: /* glsl */ `
    uniform sampler2D uTexture;
    uniform float uRefractionStrength;
    uniform float uOpacity;
    uniform float uChromaticDispersion;
    uniform float uTime;

    varying vec2 vUv;
    varying vec2 vScreenUv;
    varying vec3 vNormal;
    varying vec3 vViewPosition;

    void main() {
      vec3 normal = normalize(vNormal);
      vec3 viewDir = normalize(vViewPosition);

      // Stronger refraction for thick liquid (like water/glass)
      float offsetScale = uRefractionStrength * 0.15;
      vec2 distortion = normal.xy * offsetScale;

      // RGB split for chromatic aberration through thick liquid
      float dispersion = uChromaticDispersion * 0.5;
      vec2 baseUv = vScreenUv;
      vec2 uvR = baseUv + distortion * (1.0 + dispersion);
      vec2 uvG = baseUv + distortion;
      vec2 uvB = baseUv + distortion * (1.0 - dispersion);

      // Sample webcam texture
      float r = texture2D(uTexture, clamp(uvR, 0.001, 0.999)).r;
      float g = texture2D(uTexture, clamp(uvG, 0.001, 0.999)).g;
      float b = texture2D(uTexture, clamp(uvB, 0.001, 0.999)).b;
      
      // Base liquid color (slightly tinted cyan/blue for water feel)
      vec3 liquidTint = vec3(0.9, 0.95, 1.0);
      vec3 refractedColor = vec3(r, g, b) * liquidTint;

      // Fresnel for mirror-like edges
      float fresnel = pow(1.0 - max(dot(viewDir, normal), 0.0), 3.0);
      
      // Intense specular highlight for glossy liquid
      vec3 lightDir = normalize(vec3(0.8, 1.0, 0.5));
      vec3 halfDir = normalize(lightDir + viewDir);
      float spec = pow(max(dot(normal, halfDir), 0.0), 64.0);
      vec3 specularColor = vec3(1.0, 1.0, 1.0) * spec * 1.5; // Bright highlight

      // Combine
      vec3 finalColor = mix(refractedColor, vec3(0.8, 0.9, 1.0), fresnel * 0.4);
      finalColor += specularColor;

      gl_FragColor = vec4(finalColor, uOpacity);
    }
  `
};

export const ThermalVisionShader = {
  uniforms: {
    uTexture: { value: null },
    uOpacity: { value: 1.0 },
    // Uniforms reused so UI sliders don't crash
    uRefractionStrength: { value: 0.0 },
    uWaveRipple: { value: 0.0 },
    uTime: { value: 0.0 },
    uResolution: { value: [1.0, 1.0] },
    uIridescence: { value: 0.0 }, 
    uChromaticDispersion: { value: 0.0 }
  },

  vertexShader: /* glsl */ `
    varying vec2 vUv;
    varying vec2 vScreenUv;
    void main() {
      vUv = uv;
      // Flat projection, no 3D distortion for thermal vision to match the screenshots
      gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
      vScreenUv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
    }
  `,

  fragmentShader: /* glsl */ `
    uniform sampler2D uTexture;
    uniform float uOpacity;
    varying vec2 vUv;
    varying vec2 vScreenUv;

    // Heatmap gradient mapping function
    // Colors go from Dark Blue -> Green -> Yellow -> Red -> Pink/White
    vec3 getThermalColor(float value) {
      // Clamp the input value to ensure it stays in bounds
      float t = clamp(value, 0.0, 1.0);
      
      vec3 c0 = vec3(0.0, 0.0, 0.5);       // Dark Blue (Coldest)
      vec3 c1 = vec3(0.0, 0.8, 0.2);       // Green
      vec3 c2 = vec3(0.9, 0.9, 0.1);       // Yellow
      vec3 c3 = vec3(1.0, 0.1, 0.0);       // Red
      vec3 c4 = vec3(1.0, 0.0, 0.8);       // Pink (Hottest)
      vec3 c5 = vec3(1.0, 1.0, 1.0);       // White (Core)

      vec3 color;
      if (t < 0.2) {
        color = mix(c0, c1, t / 0.2);
      } else if (t < 0.4) {
        color = mix(c1, c2, (t - 0.2) / 0.2);
      } else if (t < 0.6) {
        color = mix(c2, c3, (t - 0.4) / 0.2);
      } else if (t < 0.8) {
        color = mix(c3, c4, (t - 0.6) / 0.2);
      } else {
        color = mix(c4, c5, (t - 0.8) / 0.2);
      }
      return color;
    }

    void main() {
      // Sample the original camera texture using screen space coordinates
      vec4 texColor = texture2D(uTexture, vScreenUv);
      
      // Calculate luminance (brightness) to act as heat
      // Using standard perceptual luminance weights
      float luminance = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
      
      // Enhance contrast slightly for better thermal effect
      float heat = smoothstep(0.1, 0.9, luminance);

      // Map the brightness to the thermal color gradient
      vec3 thermalColor = getThermalColor(heat);

      gl_FragColor = vec4(thermalColor, uOpacity);
    }
  `
};

export const AsciiShader = {
  uniforms: {
    uTexture: { value: null },
    uOpacity: { value: 1.0 },
    // Uniforms reused so UI sliders don't crash
    uRefractionStrength: { value: 0.0 },
    uWaveRipple: { value: 0.0 },
    uTime: { value: 0.0 },
    uResolution: { value: [1.0, 1.0] },
    uIridescence: { value: 0.0 }, 
    uChromaticDispersion: { value: 0.0 }
  },

  vertexShader: /* glsl */ `
    varying vec2 vScreenUv;
    void main() {
      gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
      vScreenUv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
    }
  `,

  fragmentShader: /* glsl */ `
    uniform sampler2D uTexture;
    uniform float uOpacity;
    uniform float uTime;
    varying vec2 vScreenUv;

    // ASCII procedural character generator using SDF
    float character(vec2 p, float lum) {
      // Scale coordinates to 0..1 inside the cell
      p = fract(p);
      float c = 0.0;
      
      // Determine which shape to draw based on brightness
      if (lum > 0.8) {
        // '#' Hash - Brightest
        float h = step(abs(p.x - 0.3), 0.1) + step(abs(p.x - 0.7), 0.1);
        float v = step(abs(p.y - 0.3), 0.1) + step(abs(p.y - 0.7), 0.1);
        c = clamp(h + v, 0.0, 1.0);
      } else if (lum > 0.6) {
        // '*' Asterisk
        float cross = step(abs(p.x - 0.5), 0.1) + step(abs(p.y - 0.5), 0.1);
        float diag = step(abs(p.x - p.y), 0.1) + step(abs(p.x + p.y - 1.0), 0.1);
        c = clamp(cross + diag, 0.0, 1.0);
      } else if (lum > 0.4) {
        // '+' Plus
        c = clamp(step(abs(p.x - 0.5), 0.1) + step(abs(p.y - 0.5), 0.1), 0.0, 1.0);
      } else if (lum > 0.2) {
        // '.' Dot
        c = step(length(p - vec2(0.5)), 0.15);
      } else {
        // ' ' Space - Darkest
        c = 0.0;
      }
      return c;
    }

    void main() {
      // Determine the size of the ASCII cells
      // We divide the screen into 80 columns (like a terminal)
      float cols = 80.0;
      
      // Aspect ratio of the cell (assuming 16:9 for the camera feed, cell width/height)
      vec2 cellSize = vec2(1.0 / cols, (1.0 / cols) * (16.0/9.0)); 
      
      // Calculate which cell this pixel belongs to
      vec2 cellPos = floor(vScreenUv / cellSize) * cellSize;
      
      // Sample the texture color at the center of the cell
      vec2 samplePos = cellPos + (cellSize * 0.5);
      vec4 texColor = texture2D(uTexture, samplePos);
      
      // Calculate luminance of the cell
      float luminance = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
      
      // Create character based on local coordinate inside the cell
      vec2 localPos = vScreenUv / cellSize;
      float asciiMask = character(localPos, luminance);
      
      // Matrix green color theme
      vec3 matrixGreen = vec3(0.1, 0.9, 0.2);
      
      // Optional: blend the original color slightly
      vec3 finalColor = mix(vec3(0.0), matrixGreen * (0.5 + luminance * 0.5), asciiMask);

      gl_FragColor = vec4(finalColor, uOpacity);
    }
  `
};

export const GlitchShader = {
  uniforms: {
    uTexture: { value: null },
    uTime: { value: 0.0 },
    uOpacity: { value: 1.0 },
    uResolution: { value: [1.0, 1.0] },
    uRefractionStrength: { value: 0.0 },
    uWaveRipple: { value: 0.0 },
    uIridescence: { value: 0.0 },
    uChromaticDispersion: { value: 0.0 }
  },

  vertexShader: /* glsl */ `
    varying vec2 vUv;
    varying vec2 vScreenUv;

    void main() {
      vUv = uv;
      vec4 mvPosition = viewMatrix * modelMatrix * vec4(position, 1.0);
      gl_Position = projectionMatrix * mvPosition;
      vScreenUv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
    }
  `,

  fragmentShader: /* glsl */ `
    uniform sampler2D uTexture;
    uniform float uTime;
    uniform float uOpacity;
    
    varying vec2 vUv;
    varying vec2 vScreenUv;

    // Pseudo-random generator
    float random(vec2 st) {
      return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
    }

    void main() {
      vec2 uv = vScreenUv;
      
      // Neuromancer Vibe: "The sky above the port was the color of television, tuned to a dead channel."
      
      // 1. Cyberdeck blocky glitch & tearing
      float intensity = smoothstep(0.4, 0.9, sin(uTime * 2.0)) * 0.15;
      float block = random(floor(uv * vec2(8.0, 40.0)) + uTime) * intensity;
      float tear = step(0.95, random(vec2(uTime, floor(uv.y * 15.0))));
      uv.x += (block + tear * 0.05) * step(0.5, random(vec2(uTime)));

      // 2. Harsh Chromatic Aberration (Cyan & Magenta split)
      float r = texture2D(uTexture, uv + vec2(0.015, 0.0)).r; 
      float g = texture2D(uTexture, uv).g;
      float b = texture2D(uTexture, uv - vec2(0.015, 0.0)).b;
      
      // 3. Dead Channel Monochrome base
      float lum = dot(vec3(r, g, b), vec3(0.299, 0.587, 0.114));
      vec3 deadChannel = vec3(lum);
      
      // Enhance the chromatic edges for a neon feel
      vec3 neonEdges = vec3(r, g * 0.5, b); 
      
      // 4. Cyberspace Grid Overlay
      // We use the fabric's 3D UV (vUv) so the grid sticks and warps with the mesh
      vec2 gridUv = fract(vUv * 15.0);
      float gridLines = step(0.9, gridUv.x) + step(0.9, gridUv.y);
      vec3 gridColor = vec3(0.0, 1.0, 0.8) * gridLines; // Neon Cyan
      
      // 5. TV Static Snow
      float staticNoise = random(uv + fract(uTime)) * 0.25;
      
      // Combine it all
      vec3 finalColor = mix(deadChannel, neonEdges, 0.5) + staticNoise + (gridColor * 0.8);
      
      // 6. Hard CRT Scanlines
      float scanline = sin(uv.y * 1000.0) * 0.08;
      finalColor -= scanline;

      gl_FragColor = vec4(finalColor, uOpacity);
    }
  `
};

export const CircleStackShader = {
  uniforms: {
    uTexture: { value: null },
    uTime: { value: 0.0 },
    uOpacity: { value: 1.0 },
    uResolution: { value: [1.0, 1.0] },
    uRefractionStrength: { value: 0.0 },
    uWaveRipple: { value: 0.0 },
    uIridescence: { value: 0.0 },
    uChromaticDispersion: { value: 0.0 },
    uPcbTexture: { value: null }
  },

  vertexShader: /* glsl */ `
    attribute float aEffectType; // Instanced attribute
    varying vec2 vUv;
    varying vec2 vScreenUv;
    varying float vEffectType;
    
    void main() {
      vEffectType = aEffectType;
      vUv = uv;
      
      vec4 mvPosition = viewMatrix * modelMatrix * instanceMatrix * vec4(position, 1.0);
      gl_Position = projectionMatrix * mvPosition;
      vScreenUv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
    }
  `,

  fragmentShader: /* glsl */ `
    uniform sampler2D uTexture;
    uniform float uTime;
    
    varying vec2 vUv;
    varying vec2 vScreenUv;
    varying float vEffectType;

    void main() {
      vec3 color = texture2D(uTexture, vScreenUv).rgb;
      
      int effect = int(vEffectType + 0.5);
      
      if (effect == 0) {
        // Normal Color
      } else if (effect == 1) {
        // Thermal
        float lum = dot(color, vec3(0.299, 0.587, 0.114));
        color = mix(vec3(0.0, 0.0, 0.8), vec3(1.0, 1.0, 0.0), lum);
        color = mix(color, vec3(1.0, 0.0, 0.0), smoothstep(0.5, 1.0, lum));
      } else if (effect == 2) {
        // Glitch Chromatic
        vec2 glitchUv = vScreenUv;
        float wave = sin(vScreenUv.y * 50.0 + uTime * 10.0) * 0.02;
        glitchUv.x += wave;
        color = texture2D(uTexture, glitchUv).rgb;
        color.r = texture2D(uTexture, glitchUv + vec2(0.015, 0.0)).r;
        color.b = texture2D(uTexture, glitchUv - vec2(0.015, 0.0)).b;
      } else if (effect == 3) {
        // Invert
        color = 1.0 - color;
      } else if (effect == 4) {
        // Matrix Green
        float lum = dot(color, vec3(0.299, 0.587, 0.114));
        color = vec3(0.1, lum * 1.5, 0.2);
      } else if (effect == 5) {
        // B&W High Contrast
        float lum = dot(color, vec3(0.299, 0.587, 0.114));
        color = vec3(smoothstep(0.2, 0.8, lum));
      }
      
      // Thick white border to match the TouchDesigner screenshot
      float thickness = 0.03;
      float borderMask = step(thickness, vUv.x) * step(thickness, vUv.y) * step(vUv.x, 1.0 - thickness) * step(vUv.y, 1.0 - thickness);
      
      // Inner shadow/border
      float innerBorder = step(thickness * 2.0, vUv.x) * step(thickness * 2.0, vUv.y) * step(vUv.x, 1.0 - thickness * 2.0) * step(vUv.y, 1.0 - thickness * 2.0);
      
      // If outside inner border, maybe black, else if outside border, white
      if (borderMask < 0.5) {
        color = vec3(1.0); // White frame
      } else if (innerBorder < 0.5) {
        color = vec3(0.0); // Black inner frame
      }
      
      gl_FragColor = vec4(color, 1.0);
    }
  `
};

export const TouchDesignerPortalShader = {
  uniforms: {
    uTexture: { value: null },
    uTime: { value: 0.0 },
    uOpacity: { value: 1.0 },
    uResolution: { value: [1.0, 1.0] },
    uRefractionStrength: { value: 0.5 },
    uWaveRipple: { value: 0.2 },
    uIridescence: { value: 1.0 },
    uChromaticDispersion: { value: 0.08 }
  },

  vertexShader: /* glsl */ `
    varying vec2 vUv;
    varying vec2 vScreenUv;
    varying vec3 vNormal;

    void main() {
      vUv = uv;
      vNormal = normalize(normalMatrix * normal);
      vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);
      gl_Position = projectionMatrix * mvPosition;
      vScreenUv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
    }
  `,

  fragmentShader: /* glsl */ `
    uniform sampler2D uTexture;
    uniform float uTime;
    uniform float uOpacity;
    uniform float uIridescence;
    uniform float uChromaticDispersion;
    uniform float uRefractionStrength;

    varying vec2 vUv;
    varying vec2 vScreenUv;

    // Psychedelic Spectral Rainbow Color Lookup Table (TouchDesigner style)
    vec3 psychedelicPalette(float t) {
      vec3 a = vec3(0.5, 0.5, 0.5);
      vec3 b = vec3(0.5, 0.5, 0.5);
      vec3 c = vec3(2.0, 1.0, 1.0);
      vec3 d = vec3(0.00, 0.33, 0.67);
      
      vec3 color = a + b * cos(6.28318 * (c * t + d + vec3(uTime * 0.05)));
      return color;
    }

    // Sobel edge detector for glowing high-frequency outlines
    float detectEdges(vec2 uv, vec2 stepSize) {
      float t0 = dot(texture2D(uTexture, uv + vec2(-stepSize.x, -stepSize.y)).rgb, vec3(0.299, 0.587, 0.114));
      float t1 = dot(texture2D(uTexture, uv + vec2( 0.0,        -stepSize.y)).rgb, vec3(0.299, 0.587, 0.114));
      float t2 = dot(texture2D(uTexture, uv + vec2( stepSize.x, -stepSize.y)).rgb, vec3(0.299, 0.587, 0.114));

      float t3 = dot(texture2D(uTexture, uv + vec2(-stepSize.x,  0.0)).rgb, vec3(0.299, 0.587, 0.114));
      float t5 = dot(texture2D(uTexture, uv + vec2( stepSize.x,  0.0)).rgb, vec3(0.299, 0.587, 0.114));

      float t6 = dot(texture2D(uTexture, uv + vec2(-stepSize.x,  stepSize.y)).rgb, vec3(0.299, 0.587, 0.114));
      float t7 = dot(texture2D(uTexture, uv + vec2( 0.0,         stepSize.y)).rgb, vec3(0.299, 0.587, 0.114));
      float t8 = dot(texture2D(uTexture, uv + vec2( stepSize.x,  stepSize.y)).rgb, vec3(0.299, 0.587, 0.114));

      float gx = -t0 - 2.0*t3 - t6 + t2 + 2.0*t5 + t8;
      float gy = -t0 - 2.0*t1 - t2 + t6 + 2.0*t7 + t8;

      return sqrt(gx * gx + gy * gy);
    }

    void main() {
      vec2 uv = vScreenUv;

      // Chromatic dispersion shift
      float disp = uChromaticDispersion * 0.03;
      float r = texture2D(uTexture, uv + vec2(disp, 0.0)).r;
      float g = texture2D(uTexture, uv).g;
      float b = texture2D(uTexture, uv - vec2(disp, 0.0)).b;

      vec3 texColor = vec3(r, g, b);

      // Compute luminance and solarized heat value
      float lum = dot(texColor, vec3(0.299, 0.587, 0.114));
      
      // TouchDesigner solarization curves (sine/cosine waves over luminance)
      float solarizedLum = sin(lum * 3.14159 * 2.5 + uTime * 0.2) * 0.5 + 0.5;
      
      // Pass through psychedelic rainbow palette
      vec3 rainbowColor = psychedelicPalette(solarizedLum * 1.5 + lum * 0.5);
      
      // High-contrast saturation boost
      rainbowColor = pow(rainbowColor, vec3(0.85));

      // Sobel Edge Detection for neon outlines
      vec2 stepSize = vec2(1.0 / 1280.0, 1.0 / 720.0);
      float edges = detectEdges(uv, stepSize);
      edges = smoothstep(0.12, 0.45, edges);

      // Neon outline color (bright cyan/yellow)
      vec3 edgeColor = mix(vec3(0.0, 1.0, 0.9), vec3(1.0, 0.9, 0.0), sin(uTime * 2.0) * 0.5 + 0.5);

      // Combine base rainbow heatmap with neon edge highlights
      vec3 finalColor = mix(rainbowColor, edgeColor, edges * 0.7);

      // TouchDesigner node window frame border (outer white, inner dark)
      float thickness = 0.015;
      float outerBorder = step(thickness, vUv.x) * step(thickness, vUv.y) * step(vUv.x, 1.0 - thickness) * step(vUv.y, 1.0 - thickness);
      float innerBorder = step(thickness * 1.8, vUv.x) * step(thickness * 1.8, vUv.y) * step(vUv.x, 1.0 - thickness * 1.8) * step(vUv.y, 1.0 - thickness * 1.8);

      if (outerBorder < 0.5) {
        finalColor = vec3(0.9, 0.95, 1.0); // Bright TouchDesigner node border frame
      } else if (innerBorder < 0.5) {
        finalColor = vec3(0.08, 0.1, 0.14); // Dark inner frame edge
      }

      gl_FragColor = vec4(finalColor, uOpacity);
    }
  `
};
