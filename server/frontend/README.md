# SenseMate Frontend

> **Reliable IoT (RIOT) Team — PO-2026**
> A university project for intelligent flood disaster management

## Overview

SenseMate is a **flood gate monitoring and control dashboard** that allows operators to:

- Monitor the real-time status of flood gates across Hamburg
- Open, close, or mark gates as out-of-service remotely
- View gate locations on an interactive Leaflet map
- Send downlink commands to IoT end-node devices
- Track gate activity in a chronological log
- Receive real-time notifications via WebSocket
- Support multiple user roles: **Controllers** (full control), **Viewers** (read-only), and **Guests** (unauthenticated read-only)

The frontend communicates with a Java Spring Boot backend over REST APIs and STOMP/WebSocket for real-time updates.

React Compiler is enabled for automatic component memoization at compile time — see the [documentation](docs/techstack.md) for details.

## Getting Started

### Prerequisites
- Node.js (compatible with the project's package.json engines)
- A running backend server at `http://localhost:8080`

### Install & Run
```bash
npm install
npm start
# Dev server starts at http://localhost:5173
```

### Other Commands
```bash
npm run build          # Production build to build/ folder
npm run serve          # Preview production build
npm test               # Run tests with Vitest
npm run docs           # Serve documentation at localhost:3000
```

## User Flow

1. Users start at the **Landing Page** (`/`) — choose Login, Sign Up, or Continue as Guest
2. After login/registration, users are redirected based on their role:
   - **Controller** → `/dashboard` — full gate control (CRUD, downlinks, bulk operations)
   - **Viewer** → `/dashboard` — read-only dashboard with notifications
3. **Guests** go directly to `/dashboard-guest` — read-only, no auth required

## Directory Structure

```
server/frontend/
├── public/                          # Static assets (favicon, logos, manifest)
├── src/
│   ├── app/
│   │   ├── App.jsx                  # Root component: BrowserRouter + routes with guards
│   │   ├── store.js                 # Redux store configuration
│   │   └── App.test.jsx             # Smoke test (renders landing page)
│   ├── index.jsx                    # Entry point: renders <App /> in React.StrictMode
│   ├── index.css                    # Global styles
│   ├── assets/img/                  # Image assets
│   ├── pages/                       # Top-level page components (1 per route)
│   │   ├── LandingPage.jsx          # / — hero page with CTAs
│   │   ├── LoginPage.jsx            # /login — email/password form
│   │   ├── RegisterPage.jsx         # /register — registration with role toggle
│   │   ├── DashboardPage.jsx        # /dashboard — controller/viewer dashboard
│   │   ├── DashboardGuestPage.jsx   # /dashboard-guest — guest view
│   │   ├── MapPage.jsx              # /map — Leaflet map
│   │   ├── DiagnosticsPage.jsx      # /diagnostics — system diagnostics
│   │   ├── DevicesPage.jsx          # /devices — device management
│   │   ├── AutomationPage.jsx       # /automation — automation rules
│   │   ├── LogsPage.jsx             # /logs — activity logs
│   │   └── SettingsPage.jsx         # /settings — user settings
│   ├── features/                    # Domain-driven feature modules
│   │   ├── auth/                    # Authentication: authSlice, ProtectedRoute, PublicOnlyRoute, LogoutButton
│   │   ├── gates/                   # Gate CRUD, status, priority, downlink, StatCards
│   │   ├── map/                     # Leaflet map visualization (centered on Hamburg)
│   │   ├── activities/              # Gate activity log + ActivityPanel component
│   │   └── notifications/           # Notification API + NotificationPopup
│   └── shared/                      # Cross-cutting code
│       ├── components/              # DarkModeToggle, CollapseToggle, ComingSoonHero, SkeletonLoader, UplinkToast
│       ├── styles/                  # theme.css, typography.css
│       └── utils/cookie.js          # getCookie helper
├── docs/                            # Docsify documentation site
├── index.html                       # HTML entry point
├── vite.config.js                   # Vite config (outDir: build/, global: window polyfill)
├── vitest.config.js                 # Vitest config (jsdom, globals, setup file)
└── vitest.setup.js                  # Test setup (polyfills, jest-dom matchers, cleanup)
```

## Tech Stack

| Technology | Version | Purpose |
|---|---|---|
| React | ^19.1.0 | UI framework |
| Vite | ^8.0.10 | Build tool & dev server |
| React Router DOM | ^7.6.0 | Client-side routing |
| MUI (Material UI) | ^7.1.1 | Component library + icons |
| @emotion/react + styled | ^11.14.0 | CSS-in-JS (MUI dependency) |
| react-icons | ^5.5.0 | Supplementary icons |
| Axios | ^1.9.0 | HTTP client |
| @stomp/stompjs | ^7.1.1 | WebSocket real-time updates |
| Leaflet + react-leaflet | ^1.9.4 / ^5.0.0 | Map visualization |
| Vitest | ^4.1.5 | Testing framework |

## Key Conventions

- **Plain JavaScript only** — No TypeScript; all components use `.jsx` extension
- **Feature-based folder structure** — Code organized by domain, not by technical layer
- **Barrel exports** — Each feature has an `index.js` that re-exports its public API
- **No cross-feature imports** — Features import from `shared/` but not from each other
- **Redux Toolkit + RTK Query** for auth and server state
- **Route-level guards** — `ProtectedRoute` (role-gated) and `PublicOnlyRoute`

## Documentation

Full documentation is available via docsify:

```bash
npm run docs     # Serve at localhost:3000
```

Docs are located in the `docs/` directory:
- [Project Overview](docs/overview.md)
- [Tech Stack](docs/techstack.md)
- [Architecture](docs/architecture.md)
- [Pages & Routing](docs/routing.md)
- [Authentication Flow](docs/authentication.md)
- [Real-Time Communication](docs/real-time.md)
- [Data Flow Patterns](docs/data-flow.md)
- [Build & Deployment](docs/deployment.md)
