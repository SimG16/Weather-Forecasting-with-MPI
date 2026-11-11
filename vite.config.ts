import { defineConfig } from 'vite';

export default defineConfig({
  server: {
    proxy: {
      '/forecast': {
        target: 'http://localhost:8080', // Backend URL
        changeOrigin: true,
        secure: false,
      },
      '/update_forecast': {
        target: 'http://localhost:8080',
        changeOrigin: true,
        secure: false,
      },
    },
  },
});
