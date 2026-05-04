import { defineConfig } from 'vitest/config'
import { nodePolyfills } from 'vite-plugin-node-polyfills'

// vitest.config.ts
export default defineConfig({
    test: {
        globals: true, // This is the crucial part
        environment: 'jsdom',
        setupFiles: ['./vitest.setup.js'],
    },
})