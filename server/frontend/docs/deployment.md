# Build & Deployment

## Development

```bash
# Start the Vite development server with HMR
npm start
# → Starts on http://localhost:5173 (default Vite port)
```

The dev server provides:
- Hot Module Replacement (HMR) for instant feedback
- Automatic JSX transformation via `@vitejs/plugin-react`
- Fast startup with esbuild pre-bundling

## Production Build

```bash
# Create an optimized production build
npm run build
# → Output written to build/
```

The build process:
1. Bundles all JSX/JS into optimized chunks
2. Extracts and minifies CSS
3. Copies static assets from `public/` to `build/`
4. Generates hashed filenames for cache busting
5. Outputs everything to the `build/` directory

### Build Output Structure

```
build/
├── index.html              # Entry HTML (inlined scripts)
├── assets/
│   ├── index-abc123.js     # Main JS bundle (hashed)
│   ├── index-xyz456.css    # Main CSS bundle (hashed)
│   └── ...                  # Other chunked assets
├── manifest.json
├── favicon.ico
├── logo192.png
├── logo512.png
└── robots.txt
```

## Preview Production Build

```bash
# Serve the production build locally for testing
npm run serve
# → Starts a static preview server for build/
```

## Running Tests

```bash
# Run all tests with Vitest
npm test

# Test configuration (vitest.config.js):
# - Environment: jsdom (browser simulation)
# - Global test APIs enabled
# - Setup file: vitest.setup.js (polyfills + jest-dom matchers)
```

## Configuration Files

| File | Purpose |
|---|---|
| `vite.config.js` | Build options (outDir, plugins, define) |
| `vitest.config.js` | Test runner configuration |
| `vitest.setup.js` | Test environment setup (polyfills, matchers) |
| `index.html` | HTML entry point and Vite script tag |
| `.gitignore` | Ignored files (node_modules, build, .env) |

## Environment Variables

No `.env` files are committed. The following are git-ignored:
- `.env.local`
- `.env.development.local`
- `.env.test.local`
- `.env.production.local`

**Hardcoded configuration values in source code:**

| Value | Location |
|---|---|
| `API_BASE_URL = 'http://localhost:8080'` | `src/shared/api/apiClient.js` |
| `WebSocket URL = 'http://localhost:8080/ws'` | Multiple components |
| `Admin password = "secret123"` | `src/features/gates/components/StatusTables.jsx` |

> ⚠️ These values should ideally be moved to environment variables for different deployment targets.

## Deployment Checklist

1. **Build**: Run `npm run build`
2. **Serve**: Deploy the `build/` directory to any static file server
3. **API URL**: Update `baseURL` in `apiClient.js` for production backend
4. **WebSocket URL**: Update WS URL in components for production
5. **Browser Support**: Configured via `browserslist` in `package.json`
   - Production: `>0.2%, not dead, not op_mini all`
   - Development: `last 1 chrome version, last 1 firefox version, last 1 safari version`

## Browser Support

| Environment | Browsers |
|---|---|
| Production | All browsers with >0.2% global usage (excluding dead browsers and Opera Mini) |
| Development | Latest Chrome, Firefox, Safari |

## Known Technical Debt

1. **Hardcoded API URLs** — Should use environment variables (`VITE_API_URL`) for different environments
2. **Hardcoded admin password** — Should be server-side only
3. **No service worker** — PWA manifest exists but no offline support
4. **No lazy loading** — All routes eagerly loaded; `React.lazy()` could improve initial load time
5. **Minimal test coverage** — Only a single smoke test exists
