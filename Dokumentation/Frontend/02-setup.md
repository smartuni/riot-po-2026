# 02 — Setup

## Prerequisites

- **Node.js** (version 18 or later recommended)
- **npm** (comes with Node.js)
- The **Backend** must be running for the Frontend to work (serves REST API on port 8080)

Verify your installation:

```bash
node --version    # e.g., v20.11.0
npm --version     # e.g., 10.2.4
```

---

## Installation

Navigate to the frontend directory and install dependencies:

```bash
cd server/frontend
npm install
```

This reads `server/frontend/package.json:1` and downloads all required packages into `node_modules/`.

---

## Development Server

Start the Vite dev server with Hot Module Replacement (HMR):

```bash
npm start
```

- Opens at **http://localhost:5173**
- Automatically proxies `/api` requests to `http://localhost:8080` (see [Vite Proxy Configuration](#vite-proxy-configuration))
- Automatically proxies `/ws` WebSocket connections to `http://localhost:8080`
- Changes to source files appear instantly in the browser — no manual refresh needed
- The server binds to `0.0.0.0`, making it accessible from other devices on the same network

---

## Production Build

Create an optimized production build:

```bash
npm run build
```

- Outputs bundled, minified, and hashed files to the `build/` directory
- Uses the React Compiler for automatic code optimization
- Configuration: `server/frontend/vite.config.js:1`

Preview the production build locally:

```bash
npm run serve
```

This starts a static file server at `http://localhost:4173` serving the `build/` directory.

---

## Running Tests

### Unit Tests (Vitest)

```bash
npm test
```

- Uses **Vitest** (v4.x), a Vite-native test runner compatible with Jest APIs
- Tests run in a simulated browser environment (**jsdom**)
- Configuration: `server/frontend/vitest.setup.js` and `server/frontend/vitest.config.js`
- Example test files: `server/frontend/src/app/App.test.jsx:1`, `server/frontend/src/features/health/components/HealthBadge.test.jsx:1`

### End-to-End Tests (Playwright)

```bash
npm run test:e2e
```

- Opens a real Chromium browser, clicks through the app, and verifies behavior
- Test files live in `server/frontend/tests/`
- Requires the Backend to be running on `http://localhost:8080`
- Playwright can auto-start the backend via Docker Compose (e2e profile)
- Interactive UI mode: `npm run test:e2e:ui`

---

## Linting

Check code quality and catch potential errors:

```bash
npm run lint
```

- Uses **ESLint** (v9.x)
- Scans all files in `server/frontend/src/`
- Enforces React Compiler rules at "error" severity

---

## Vite Proxy Configuration

Defined in `server/frontend/vite.config.js:13`:

```javascript
server: {
    host: '0.0.0.0',
    proxy: {
        '/api': {
            target: 'http://localhost:8080',
            changeOrigin: true,
            rewrite: (path) => path.replace(/^\/api/, ''),
        },
        '/ws': {
            target: 'http://localhost:8080',
            ws: true,
        },
    },
},
```

**What this means:**
- When the browser requests `http://localhost:5173/api/gates`, Vite intercepts it, strips the `/api` prefix, and forwards it to `http://localhost:8080/gates` on the backend.
- When the browser requests `ws://localhost:5173/ws`, Vite forwards the WebSocket connection to `ws://localhost:8080/ws` on the backend.
- This avoids CORS (Cross-Origin Resource Sharing) issues during development — the browser thinks everything comes from the same origin.

---

## Environment Variables

The Frontend uses one optional environment variable for production deployments:

| Variable | Default | Purpose |
|----------|---------|---------|
| `VITE_API_BASE_URL` | `''` (empty = relative path) | Override the backend URL for API calls |

Set it in a `.env.local` file (git-ignored):

```bash
VITE_API_BASE_URL=https://api.example.com
```

Used in `server/frontend/src/app/store/api/api.js:5` and `server/frontend/src/app/store/slices/authSlice.js:6`.

---

## Quick-Start Checklist

1. Ensure the Backend is running (`./mvnw spring-boot:run -Dspring-boot.run.profiles=e2e` for quick H2-based setup)
2. `cd server/frontend && npm install`
3. `npm start` — open http://localhost:5173
4. Log in with test credentials: `test@example.com` / `test123` (controller) or `test2@example.com` / `test234` (viewer)
