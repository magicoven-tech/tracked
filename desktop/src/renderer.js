import * as THREE from 'three';
import { IridescentFabricShader, LiquidRippleShader, ThermalVisionShader, AsciiShader, GlitchShader, CircleStackShader, TouchDesignerPortalShader } from './shaders.js';
import { MeshBuilder } from './meshBuilder.js';

// MediaPipe Hand Landmark Connections
const HAND_CONNECTIONS = [
  [0, 1], [1, 2], [2, 3], [3, 4],           // Thumb
  [0, 5], [5, 6], [6, 7], [7, 8],           // Index
  [5, 9], [9, 10], [10, 11], [11, 12],      // Middle
  [9, 13], [13, 14], [14, 15], [15, 16],    // Ring
  [13, 17], [0, 17], [17, 18], [18, 19], [19, 20] // Pinky
];

export class AppRenderer {
  constructor(canvas3d, skeletonCanvas, videoElement) {
    this.canvas3d = canvas3d;
    this.skeletonCanvas = skeletonCanvas;
    this.ctx2d = skeletonCanvas.getContext('2d');
    this.video = videoElement;

    // Three.js Core
    this.scene = new THREE.Scene();
    this.camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 100);
    this.camera.position.z = 3.0;

    this.renderer = new THREE.WebGLRenderer({
      canvas: canvas3d,
      antialias: true,
      alpha: true,
      preserveDrawingBuffer: true
    });
    this.renderer.setSize(window.innerWidth, window.innerHeight);
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));

    // Video Texture
    this.videoTexture = new THREE.VideoTexture(this.video);
    this.videoTexture.minFilter = THREE.LinearFilter;
    this.videoTexture.magFilter = THREE.LinearFilter;
    this.videoTexture.colorSpace = THREE.SRGBColorSpace;

    // Custom Shader Material
    this.material = new THREE.ShaderMaterial({
      vertexShader: IridescentFabricShader.vertexShader,
      fragmentShader: IridescentFabricShader.fragmentShader,
      uniforms: THREE.UniformsUtils.clone(IridescentFabricShader.uniforms),
      transparent: true,
      side: THREE.DoubleSide,
      depthWrite: false
    });
    
    // Fabric Mesh Stack Instanced Mesh (30 cards)
    const instanceCount = 30;
    const stackGeo = new THREE.PlaneGeometry(0.35, 0.35);
    this.stackMaterial = new THREE.ShaderMaterial({
      vertexShader: CircleStackShader.vertexShader,
      fragmentShader: CircleStackShader.fragmentShader,
      uniforms: THREE.UniformsUtils.clone(CircleStackShader.uniforms),
      transparent: true,
      side: THREE.DoubleSide,
      depthWrite: false
    });
    this.stackMaterial.uniforms.uTexture.value = this.videoTexture;

    this.instancedStack = new THREE.InstancedMesh(stackGeo, this.stackMaterial, instanceCount);
    
    const effectTypes = new Float32Array(instanceCount);
    for (let i = 0; i < instanceCount; i++) {
      effectTypes[i] = i % 6; // Assign a different effect index per card
    }
    stackGeo.setAttribute('aEffectType', new THREE.InstancedBufferAttribute(effectTypes, 1));
    this.instancedStack.visible = false;
    this.scene.add(this.instancedStack);

    // Connecting Line for Circle Stack
    const lineGeo = new THREE.BufferGeometry().setFromPoints([new THREE.Vector3(0,0,0), new THREE.Vector3(1,1,1)]);
    const lineMat = new THREE.LineBasicMaterial({ color: 0xffffff, linewidth: 2 });
    this.connectingLine = new THREE.Line(lineGeo, lineMat);
    this.connectingLine.visible = false;
    this.scene.add(this.connectingLine);

    // Fabric Mesh
    this.meshBuilder = new MeshBuilder(24, 24);
    this.material.uniforms.uTexture.value = this.videoTexture;

    this.fabricMesh = new THREE.Mesh(this.meshBuilder.geometry, this.material);
    this.scene.add(this.fabricMesh);

    // Video Background Plane
    const bgGeo = new THREE.PlaneGeometry(1, 1);
    const bgMat = new THREE.MeshBasicMaterial({ map: this.videoTexture });
    this.bgMesh = new THREE.Mesh(bgGeo, bgMat);
    this.bgMesh.position.z = -0.8;
    this.scene.add(this.bgMesh);

    // Settings
    this.skeletonColor = '#00ffaa';
    this.skeletonThickness = 2.5;
    this.isPlaying = true;
    this.clock = new THREE.Clock();

    this.handleResize();
    window.addEventListener('resize', () => this.handleResize());
  }

  setEffectType(type) {
    this.currentEffect = type;
    if (type === 'touchDesignerPortal') {
      this.material.vertexShader = TouchDesignerPortalShader.vertexShader;
      this.material.fragmentShader = TouchDesignerPortalShader.fragmentShader;
    } else if (type === 'liquidRipple') {
      this.material.vertexShader = LiquidRippleShader.vertexShader;
      this.material.fragmentShader = LiquidRippleShader.fragmentShader;
    } else if (type === 'thermalVision') {
      this.material.vertexShader = ThermalVisionShader.vertexShader;
      this.material.fragmentShader = ThermalVisionShader.fragmentShader;
    } else if (type === 'ascii') {
      this.material.vertexShader = AsciiShader.vertexShader;
      this.material.fragmentShader = AsciiShader.fragmentShader;
    } else if (type === 'glitch') {
      this.material.vertexShader = GlitchShader.vertexShader;
      this.material.fragmentShader = GlitchShader.fragmentShader;
    } else {
      this.material.vertexShader = IridescentFabricShader.vertexShader;
      this.material.fragmentShader = IridescentFabricShader.fragmentShader;
    }
    this.material.needsUpdate = true;
  }

  handleResize() {
    const width = window.innerWidth;
    const height = window.innerHeight;

    this.renderer.setSize(width, height);
    this.skeletonCanvas.width = width;
    this.skeletonCanvas.height = height;

    this.camera.aspect = width / height;
    this.camera.updateProjectionMatrix();

    this.updateBgMeshScale();

    if (this.material.uniforms.uResolution) {
      this.material.uniforms.uResolution.value = [width, height];
    }
  }

  updateBgMeshScale() {
    const distance = 3.8;
    const fovRad = THREE.MathUtils.degToRad(this.camera.fov);
    const visibleHeight = 2 * Math.tan(fovRad / 2) * distance;
    const visibleWidth = visibleHeight * this.camera.aspect;

    const videoAspect = (this.video.videoWidth && this.video.videoHeight)
      ? (this.video.videoWidth / this.video.videoHeight)
      : (16 / 9);

    let planeWidth = visibleWidth;
    let planeHeight = visibleHeight;

    if (this.camera.aspect < videoAspect) {
      planeWidth = visibleHeight * videoAspect;
    } else {
      planeHeight = visibleWidth / videoAspect;
    }

    this.bgMesh.scale.set(planeWidth, planeHeight, 1);
    this.visibleBounds = { width: visibleWidth, height: visibleHeight };
  }

  generateSyntheticLandmarks() {
    const leftCenter = { x: 0.35, y: 0.5, z: 0 };
    const rightCenter = { x: 0.65, y: 0.5, z: 0 };

    const generateHand = (center, isLeft) => {
      const landmarks = [];
      const dir = isLeft ? -1 : 1;
      landmarks[0] = { x: center.x, y: center.y + 0.12, z: 0 };
      const fingerAngles = [-0.5, -0.15, 0.1, 0.35, 0.6];
      let idx = 1;
      for (let f = 0; f < 5; f++) {
        const angle = fingerAngles[f] * dir;
        let px = center.x + Math.sin(angle) * 0.03 * dir;
        let py = center.y + Math.cos(angle) * 0.03;
        for (let j = 0; j < 4; j++) {
          px += Math.sin(angle) * 0.025 * dir;
          py -= Math.cos(angle) * 0.03;
          landmarks[idx++] = { x: px, y: py, z: (j * 0.01) };
        }
      }
      return landmarks;
    };

    return [generateHand(leftCenter, true), generateHand(rightCenter, false)];
  }

  updateHandLandmarks(results) {
    // If paused, freeze tracking updates and keep current frame intact
    if (!this.isPlaying) return;

    let handLandmarks = (results && results.multiHandLandmarks && results.multiHandLandmarks.length > 0)
      ? results.multiHandLandmarks
      : null;

    let isUsingFallback = false;
    if (!handLandmarks) {
      handLandmarks = this.generateSyntheticLandmarks();
      isUsingFallback = true;
    }

    const hand1 = handLandmarks[0];
    const hand2 = handLandmarks[1] || null;
    const bounds = this.visibleBounds || { width: 3.5, height: 2.0 };

    if (this.currentEffect === 'circle') {
      this.fabricMesh.visible = false;

      if (hand1 && hand2) {
        this.instancedStack.visible = true;
        this.connectingLine.visible = true;

        const p1 = this.meshBuilder.convertPoint(hand1[8], this.camera.aspect, bounds);
        const p2 = this.meshBuilder.convertPoint(hand2[8], this.camera.aspect, bounds);
        
        const v1 = new THREE.Vector3(p1.x, p1.y, p1.z);
        const v2 = new THREE.Vector3(p2.x, p2.y, p2.z);

        this.connectingLine.geometry.setFromPoints([v1, v2]);

        const dummy = new THREE.Object3D();
        const count = this.instancedStack.count;
        for (let i = 0; i < count; i++) {
          const t = i / (count - 1);
          const pos = new THREE.Vector3().lerpVectors(v1, v2, t);
          
          // Spiral chaos formula
          const angle = t * Math.PI * 6.0 + this.clock.getElapsedTime() * 3.0;
          const radius = Math.sin(t * Math.PI) * 0.4;
          
          const dir = new THREE.Vector3().subVectors(v2, v1).normalize();
          const up = new THREE.Vector3(0, 1, 0);
          let right = new THREE.Vector3().crossVectors(dir, up).normalize();
          if (right.lengthSq() < 0.001) right.set(1, 0, 0);
          const realUp = new THREE.Vector3().crossVectors(right, dir).normalize();

          pos.add(right.clone().multiplyScalar(Math.cos(angle) * radius));
          pos.add(realUp.clone().multiplyScalar(Math.sin(angle) * radius));

          dummy.position.copy(pos);
          dummy.lookAt(this.camera.position);
          dummy.rotateZ(Math.sin(i * 12.3) * 0.5); // Random tilt
          
          dummy.updateMatrix();
          this.instancedStack.setMatrixAt(i, dummy.matrix);
        }
        this.instancedStack.instanceMatrix.needsUpdate = true;
      } else {
        this.instancedStack.visible = false;
        this.connectingLine.visible = false;
      }

    } else {
      this.fabricMesh.visible = true;
      this.instancedStack.visible = false;
      this.connectingLine.visible = false;
      this.meshBuilder.updateMeshFromLandmarks(hand1, hand2, this.camera.aspect, bounds);
    }

    if (isUsingFallback) {
      this.clearSkeletonCanvas();
    } else {
      this.drawSkeletonOverlay(handLandmarks);
    }
  }

  drawSkeletonOverlay(handsLandmarks) {
    const ctx = this.ctx2d;
    const w = this.skeletonCanvas.width;
    const h = this.skeletonCanvas.height;

    ctx.clearRect(0, 0, w, h);

    ctx.strokeStyle = this.skeletonColor;
    ctx.fillStyle = this.skeletonColor;
    ctx.lineWidth = this.skeletonThickness;
    ctx.lineCap = 'round';
    ctx.lineJoin = 'round';

    handsLandmarks.forEach(landmarks => {
      HAND_CONNECTIONS.forEach(([i, j]) => {
        const p1 = landmarks[i];
        const p2 = landmarks[j];

        const x1 = (1.0 - p1.x) * w;
        const y1 = p1.y * h;
        const x2 = (1.0 - p2.x) * w;
        const y2 = p2.y * h;

        ctx.beginPath();
        ctx.moveTo(x1, y1);
        ctx.lineTo(x2, y2);
        ctx.stroke();
      });

      landmarks.forEach(p => {
        const x = (1.0 - p.x) * w;
        const y = p.y * h;

        ctx.beginPath();
        ctx.arc(x, y, this.skeletonThickness * 1.5, 0, Math.PI * 2);
        ctx.fill();
      });
    });
  }

  clearSkeletonCanvas() {
    this.ctx2d.clearRect(0, 0, this.skeletonCanvas.width, this.skeletonCanvas.height);
  }

  render() {
    if (this.isPlaying) {
      const elapsedTime = this.clock.getElapsedTime();
      this.material.uniforms.uTime.value = elapsedTime;
      if (this.stackMaterial) {
        this.stackMaterial.uniforms.uTime.value = elapsedTime;
      }

      if (this.video.readyState >= this.video.HAVE_CURRENT_DATA) {
        this.videoTexture.needsUpdate = true;
        this.updateBgMeshScale();
      }
    }

    this.renderer.render(this.scene, this.camera);
  }
}
