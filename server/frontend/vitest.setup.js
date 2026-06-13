import { expect, afterEach, vi } from 'vitest';
import { cleanup } from '@testing-library/react';
import * as matchers from '@testing-library/jest-dom/matchers';
import { Buffer } from 'node:buffer';
import process from 'node:process';

// Mock window.matchMedia (not available in jsdom)
Object.defineProperty(window, 'matchMedia', {
  writable: true,
  value: vi.fn().mockImplementation(query => ({
    matches: false,
    media: query,
    onchange: null,
    addListener: vi.fn(),
    removeListener: vi.fn(),
    addEventListener: vi.fn(),
    removeEventListener: vi.fn(),
    dispatchEvent: vi.fn(),
  })),
});

// 1. Polyfills (from our previous step)
globalThis.Buffer = Buffer;
globalThis.process = process;

// 2. Extend Vitest's "expect" with modern jest-dom matchers
expect.extend(matchers);

// 3. Cleanup after each test case (prevents memory leaks/DOM clutter)
afterEach(() => {
    cleanup();
});