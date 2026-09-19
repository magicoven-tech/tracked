import { defineConfig } from 'vite';

export default defineConfig({
  base: './', // Relative paths for Electron file:// protocol
  build: {
    outDir: 'dist',
    emptyOutDir: true,
    rollupOptions: {
      output: {
        manualChunks: {
          three: ['three']
        }
      }
    }
  },
  server: {
    host: true,
    port: 5173
  }
});
