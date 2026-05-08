# Technology Stack

## Core Framework

| Technology | Version | Purpose |
|---|---|---|
| **React** | ^19.1.0 | UI framework — component-based rendering |
| **Vite** | ^8.0.10 | Build tool & dev server (HMR, fast rebuilds) |
| **JavaScript (JSX)** | ES2022+ | Application language |

React 19 provides the latest concurrent features and automatic batching. Vite provides near-instant hot module replacement during development and optimized production builds.

## Routing

| Technology | Version | Purpose |
|---|---|---|
| **react-router-dom** | ^7.6.0 | Client-side routing with `<BrowserRouter>` |

Seven routes are defined declaratively in the root `App.jsx` component. No nested or lazy-loaded routes are used.

## UI & Styling

| Technology | Version | Purpose |
|---|---|---|
| **MUI (Material UI)** | ^7.1.1 | Component library — dialogs, buttons, tables, forms, cards |
| **MUI Icons** | ^7.1.1 | Icon set for UI controls |
| **react-icons** | ^5.5.0 | Supplementary icon library |
| **@emotion/react** | ^11.14.0 | CSS-in-JS engine (MUI dependency) |
| **@emotion/styled** | ^11.14.0 | Styled components for custom theming |
| **Plain CSS** | — | Feature-specific and shared stylesheets |

MUI provides the design system foundation. Feature-specific and shared components use plain CSS files alongside MUI's `sx` prop.

## HTTP & Real-Time Communication

| Technology | Version | Purpose |
|---|---|---|
| **Axios** | ^1.9.0 | HTTP client for REST API calls |
| **STOMP.js** | ^7.1.1 | STOMP protocol client for WebSocket messaging |
| **SockJS** | ^1.6.1 | WebSocket fallback transport |

The API layer is a single Axios instance configured with `baseURL: 'http://localhost:8080'` and JSON headers. Authorization tokens are injected imperatively after login. WebSocket connections are managed per-component via `useEffect` lifecycle hooks.

## Maps

| Technology | Version | Purpose |
|---|---|---|
| **Leaflet** | ^1.9.4 | Open-source map rendering library |
| **react-leaflet** | ^5.0.0 | React bindings for Leaflet |

The map is centered on Hamburg (53.546, 9.99) with diamond-shaped markers colored by gate status (red = open, green = closed). Markers display popups with gate details and current status.

## Testing

| Technology | Version | Purpose |
|---|---|---|
| **Vitest** | ^4.1.5 | Test runner (Vite-native, Jest-compatible) |
| **jsdom** | ^29.1.1 | Browser environment simulation |
| **@testing-library/react** | ^16.3.0 | Component testing utilities |
| **@testing-library/jest-dom** | ^6.6.0 | DOM-specific matchers |

Test configuration uses Vitest with the jsdom environment. A setup file polyfills `Buffer` and `process` for browser test compatibility and automatically cleans up DOM after each test.

## Build Configuration

```javascript
// vite.config.js — production build outputs to build/
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

export default defineConfig(() => ({
  build: { outDir: 'build' },
  plugins: [react()],
  define: { global: 'window' }, // SockJS compatibility
}));
```

The `global: 'window'` define polyfill ensures libraries like SockJS that reference the Node.js `global` object work correctly in the browser.

## Dependency Graph

```
┌─────────────────────────────────────────┐
│            react (^19.1.0)              │
├─────────────────────────────────────────┤
│  react-router-dom (^7.6.0)             │
│  react-leaflet (^5.0.0)                │
│  react-icons (^5.5.0)                  │
├─────────────────────────────────────────┤
│  @mui/material (^7.1.1)                │
│  @mui/icons-material (^7.1.1)          │
│  @emotion/react (^11.14.0)             │
│  @emotion/styled (^11.14.0)            │
├─────────────────────────────────────────┤
│  axios (^1.9.0)     │   stompjs (^7.1.1)      │
│  (REST API)         │   sockjs (^1.6.1)       │
│                     │   (WebSocket)           │
├─────────────────────────────────────────┤
│  leaflet (^1.9.4)                       │
│  (Map rendering)                        │
├─────────────────────────────────────────┤
│  vite (^8.0.10) — Build tool            │
│  vitest (^4.1.5) — Test runner          │
└─────────────────────────────────────────┘
```
