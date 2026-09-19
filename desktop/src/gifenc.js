import * as gifencModule from 'gifenc';

const pkg = (gifencModule.default && typeof gifencModule.default.quantize === 'function')
  ? gifencModule.default
  : gifencModule;

export const GIFEncoder = pkg.GIFEncoder || (typeof pkg === 'function' ? pkg : null);
export const quantize = pkg.quantize;
export const applyPalette = pkg.applyPalette;

export default GIFEncoder;
