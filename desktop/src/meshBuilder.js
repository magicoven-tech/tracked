import * as THREE from 'three';

export class MeshBuilder {
  constructor(gridCols = 24, gridRows = 24) {
    this.cols = gridCols;
    this.rows = gridRows;

    // Create PlaneGeometry with subdivs
    this.geometry = new THREE.PlaneGeometry(2, 2, this.cols - 1, this.rows - 1);
    this.positionAttr = this.geometry.attributes.position;
    this.normalAttr = this.geometry.attributes.normal;
    this.uvAttr = this.geometry.attributes.uv;

    // Smooth position interpolation buffers
    this.targetPositions = new Float32Array(this.positionAttr.count * 3);
    this.currentPositions = new Float32Array(this.positionAttr.count * 3);
  }

  updateMeshFromLandmarks(hand1Landmarks, hand2Landmarks, viewportAspect = 16 / 9, visibleBounds = { width: 3.5, height: 2.0 }) {
    if (!hand1Landmarks) return;

    // Extract border control points from Hand 1
    const p1 = this.extractHandContourPoints(hand1Landmarks, viewportAspect, visibleBounds);
    let p2;

    if (hand2Landmarks) {
      // 2 hands active: bridge between Hand 1 (Left) and Hand 2 (Right)
      p2 = this.extractHandContourPoints(hand2Landmarks, viewportAspect, visibleBounds);
    } else {
      // 1 hand active: anchor to screen edge or floating offset
      p2 = p1.map(pt => ({
        x: pt.x > 0 ? pt.x - (visibleBounds.width * 0.4) : pt.x + (visibleBounds.width * 0.4),
        y: pt.y,
        z: pt.z - 0.2
      }));
    }

    // Bilinear interpolation across grid (rows x cols)
    let idx = 0;
    for (let r = 0; r < this.rows; r++) {
      const v = r / (this.rows - 1);
      // Interpolate along Hand 1 contour for left edge
      const ptLeft = this.interpolateContour(p1, v);
      // Interpolate along Hand 2 contour for right edge
      const ptRight = this.interpolateContour(p2, v);

      for (let c = 0; c < this.cols; c++) {
        const u = c / (this.cols - 1);

        // Interpolate 3D position between left and right hand edges
        const x = THREE.MathUtils.lerp(ptLeft.x, ptRight.x, u);
        const y = THREE.MathUtils.lerp(ptLeft.y, ptRight.y, u);

        // 100% Flat geometric projection
        const z = THREE.MathUtils.lerp(ptLeft.z, ptRight.z, u);

        this.targetPositions[idx * 3] = x;
        this.targetPositions[idx * 3 + 1] = y;
        this.targetPositions[idx * 3 + 2] = z;

        idx++;
      }
    }

    // Smoothly lerp vertex positions (exponential smoothing for fluid cloth physics)
    const lerpSpeed = 0.25;
    for (let i = 0; i < this.positionAttr.count * 3; i++) {
      this.currentPositions[i] += (this.targetPositions[i] - this.currentPositions[i]) * lerpSpeed;
      this.positionAttr.array[i] = this.currentPositions[i];
    }

    this.positionAttr.needsUpdate = true;
    this.geometry.computeVertexNormals();
    this.normalAttr.needsUpdate = true;
  }

  // Convert MediaPipe landmark normalized coordinates (0..1) to Three.js camera space
  convertPoint(landmark, aspect = 16 / 9, visibleBounds = { width: 3.5, height: 2.0 }) {
    // Invert X for selfie view mirror effect
    const normX = (1.0 - landmark.x) * 2.0 - 1.0;
    const normY = -(landmark.y * 2.0 - 1.0);

    const x = normX * (visibleBounds.width * 0.35);
    const y = normY * (visibleBounds.height * 0.35);
    const z = (landmark.z || 0) * -1.5;
    return { x, y, z };
  }

  // Extract key contour points around hand perimeter (wrist, fingertips, knuckles)
  extractHandContourPoints(landmarks, aspect, visibleBounds) {
    // MediaPipe landmark indices: 8=index tip (top corner), 4=thumb tip (bottom corner)
    // This creates a quad pinned between Index finger and Thumb of both hands
    const indices = [8, 4];
    return indices.map(idx => this.convertPoint(landmarks[idx], aspect, visibleBounds));
  }

  interpolateContour(points, t) {
    const n = points.length - 1;
    const scaledT = t * n;
    const idx = Math.min(Math.floor(scaledT), n - 1);
    const subT = scaledT - idx;

    const pA = points[idx];
    const pB = points[idx + 1];

    return {
      x: THREE.MathUtils.lerp(pA.x, pB.x, subT),
      y: THREE.MathUtils.lerp(pA.y, pB.y, subT),
      z: THREE.MathUtils.lerp(pA.z, pB.z, subT)
    };
  }
}
