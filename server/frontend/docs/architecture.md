# Architecture

## High-Level Overview

The frontend follows a **feature-based SPA architecture** — purely client-side React, no SSR, communicating with a Java Spring Boot backend via REST and STOMP/WebSocket.

```mermaid
flowchart TB
    subgraph Browser["BROWSER (SPA)"]
        subgraph ReactApp["React Application"]
            Router["BrowserRouter"]
            Pages["Page Components<br/>(11 pages)"]
            
            Features["Feature Modules"]
            Features --> Router
            Features --> Pages
            
            Features --> F1[auth]
            Features --> F2[gates]
            Features --> F3[map]
            Features --> F4[activities]
            Features --> F5[notifications]
            Features --> F6[shell]
            
            Shared["Shared Layer"]
            Shared --> Components["components"]
            Shared --> Styles["styles"]
            Shared --> Utils["utils"]
            
            Features --> Shared
        end
        
        HTTP[("HTTP<br/>(RTK Query)")]
        WS[("STOMP/WS")]
        
        ReactApp --> HTTP
        ReactApp --> WS
    end
    
    Backend["Spring Boot<br/>Backend<br/>:8080"]
    
    HTTP --> Backend
    WS --> Backend
```

## Layers

### 1. App Shell (`src/app/`)

`App.jsx` is the composition root: `BrowserRouter` wraps `Routes` with page components. Protected pages are wrapped with `ProtectedRoute` (role-gated) or `PublicOnlyRoute` route guards from `features/auth/`.

### 2. Pages (`src/pages/`)

Top-level components that assemble feature components into layouts and handle auth guards. One page per route.

### 3. Features (`src/features/`)

Six domain modules, each with the same internal shape:

```
features/{feature}/
├── index.js        # Barrel exports (public API)
├── components/     # React components
└── styles/         # CSS (optional)
```

| Feature | Purpose | Components |
|---|---|---|
| `auth` | Authentication, session, user management | `LogoutButton`, `ProtectedRoute`, `PublicOnlyRoute` |
| `gates` | Gate CRUD, status control, downlinks | `StatusTables`, `StatusTablesView`, `StatCards`, `StatusChangedDialog` |
| `map` | Leaflet map visualization | `MapView` |
| `activities` | Gate activity log | `ActivityPanel` |
| `notifications` | User notification system | `NotificationPopup` |
| `shell` | Application chrome (layout, navigation) | `AppLayout`, `Sidebar`, `Topbar` |

Minimal cross-feature imports: `shell` imports from `auth` + `notifications`; `map` imports from `gates`. All other features go through `shared/`.

### 4. Shared (`src/shared/`)

Cross-cutting code:

| Module | Contents |
|---|---|
| `components/` | `DarkModeToggle`, `CollapseToggle`, `ComingSoonHero`, `SkeletonLoader`, `UplinkToast` |
| `styles/` | `theme.css`, `typography.css` |
| `utils/cookie.js` | `getCookie` |

## State Management

Redux Toolkit + RTK Query:

| State Type | Mechanism | Example |
|---|---|---|
| Auth state | Redux (`authSlice`) | User, login/logout, role |
| Server data | RTK Query (api.js) | Gate list, activities, notifications |
| UI state | Local `useState` | Dialog open/close, search, tab selection |
| Real-time data | WebSocket middleware (Redux) | Gate status changes, new activities |
| Downlink counter | RTK Query + local `useState` | Count value, rate limiting state |

## React Compiler

The project uses React Compiler to automatically memoize components at compile time, removing the need for manual optimization.

### How it works

React Compiler runs as a Babel transform (`babel-plugin-react-compiler`) during the Vite build process. It analyzes component render functions and automatically wraps expensive computations and JSX trees with memoization, similar to what developers would otherwise do manually with `useMemo` and `useCallback`.

### Build pipeline integration

```
vite.config.js → @vitejs/plugin-react({ reactCompiler: true })
                     → @rolldown/plugin-babel → babel-plugin-react-compiler
```

The compiler is enabled via the `reactCompiler` option in the `react()` plugin call in `vite.config.js`. No changes to component code were required — the compiler is purely additive.

### Impact

- No manual memoization (`useMemo`, `useCallback`, `React.memo`) is needed in the codebase
- ESLint compiler rules (via `eslint-plugin-react-hooks`) enforce safe React patterns at "error" severity
- Rollback is trivial: set `reactCompiler: false` in `vite.config.js`

## Component Dependencies

```mermaid
flowchart TB
    subgraph Pages
        DP[DashboardPage]
        DGP[DashboardGuestPage]
        LP[LoginPage]
        RP[RegisterPage]
        MP[MapPage]
        DiagP[DiagnosticsPage]
        DevP[DevicesPage]
        AutP[AutomationPage]
        LogsP[LogsPage]
        SetP[SettingsPage]
    end

    subgraph Features
        ST[StatusTables]
        STV[StatusTablesView]
        SC[StatCards]
        SCD[StatusChangedDialog]
        MV[MapView]
        AP[ActivityPanel]
        NP[NotificationPopup]
        AL[AppLayout]
        SB[Sidebar]
        TB[Topbar]
        LB[LogoutButton]
        PR[ProtectedRoute]
        POR[PublicOnlyRoute]
    end

    DP --> AL & SC & ST & AP
    DGP --> SC & STV & AP
    MP --> AL & MV
    DiagP --> AL
    DevP --> AL
    AutP --> AL
    LogsP --> AL & AP
    SetP --> AL & LB

    AL --> SB & TB
    ST --> SCD
    TB -.->|imports| notifications
    TB -.->|imports| auth
    MV -.->|imports| gates
```

Solid = composition, dashed = cross-feature import.
