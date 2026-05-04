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
   - **Viewer** → `/dashboard-view` — read-only dashboard with notifications
3. **Guests** go directly to `/dashboard-guest` — read-only, no auth required

## Directory Structure

```
server/frontend/
├── public/                          # Static assets (favicon, logos, manifest)
├── src/
│   ├── app/
│   │   ├── App.jsx                  # Root component: AuthProvider + BrowserRouter + 7 routes
│   │   └── App.test.jsx             # Smoke test (renders landing page)
│   ├── index.jsx                    # Entry point: renders <App /> in React.StrictMode
│   ├── index.css                    # Global styles
│   ├── assets/img/                  # Image assets
│   ├── pages/                       # Top-level page components (1 per route)
│   │   ├── LandingPage.jsx          # / — hero page with CTAs
│   │   ├── LoginPage.jsx            # /login — email/password form
│   │   ├── RegisterPage.jsx         # /register — registration with role toggle
│   │   ├── DashboardPage.jsx        # /dashboard — controller dashboard
│   │   ├── DashboardViewPage.jsx    # /dashboard-view — viewer dashboard
│   │   ├── DashboardGuestPage.jsx   # /dashboard-guest — guest view
│   │   └── UserPage.jsx            # /userpage — profile management + logout
│   ├── features/                    # Domain-driven feature modules
│   │   ├── auth/                    # Authentication: context, provider, API, LogoutButton
│   │   ├── gates/                   # Gate CRUD, status, priority, downlink, InfoBoxes
│   │   ├── map/                     # Leaflet map visualization (centered on Hamburg)
│   │   ├── activities/              # Gate activity log + RecentActivity component
│   │   └── notifications/           # Notification API + NotificationPopup
│   └── shared/                      # Cross-cutting code
│       ├── api/apiClient.js         # Axios singleton (baseURL: http://localhost:8080)
│       ├── components/             # AlertDialog variants, HeaderBar, HeaderBarGuest
│       ├── styles/                  # Global CSS (App.css, HeaderBar.css, Sidebar.css)
│       └── utils/cookie.js          # getCookie, setCookie, eraseCookie helpers
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
| @stomp/stompjs + sockjs-client | ^7.1.1 / ^1.6.1 | WebSocket real-time updates |
| Leaflet + react-leaflet | ^1.9.4 / ^5.0.0 | Map visualization |
| Vitest | ^4.1.5 | Testing framework |

## Key Conventions

- **Plain JavaScript only** — No TypeScript; all components use `.jsx` extension
- **Feature-based folder structure** — Code organized by domain, not by technical layer
- **Barrel exports** — Each feature has an `index.js` that re-exports its public API
- **No cross-feature imports** — Features import from `shared/` but not from each other
- **No state management library** — React Context for auth, local `useState`/`useEffect` for all other state
- **No route guards** — Auth checks done imperatively in each page component via `useEffect`

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
