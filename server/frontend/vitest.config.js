import { defineConfig } from 'vitest/config'

// vitest.config.ts
export default defineConfig({
    test: {
        globals: true, // This is the crucial part
        environment: 'jsdom',
        setupFiles: ['./vitest.setup.js'],
        exclude: ['tests/**', 'node_modules/**'],
    },
})