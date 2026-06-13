# Map Style Switcher

## TL;DR
> **Summary**: Add tile provider registry + user-facing style picker in Settings → Advanced. Make MapView's TileLayer URL dynamic from Redux state. Persist preference via API + localStorage fallback. Future: optional mapcn/MapLibre vector option.
> **Estimated Effort**: Medium

## Context
### Original Request
Replace/augment leaflet map with mapcn for better visuals. User settings page with Advanced tab to pick map style. Options: OSM (default) + mapcn-styled maps.

### Key Findings
- **Stack**: React 19 + Vite 8 + Redux Toolkit + react-leaflet 5 + leaflet 1.9.4. No Tailwind, no shadcn/ui.
- **MapView** (`features/map/components/MapView.jsx`): Hard-coded OSM `TileLayer` URL. Uses `MapContainer`, `TileLayer`, `Marker`, `Popup`. Custom `divIcon` markers. Center Hamburg, zoom 13.
- **3 consumers**: `MapPage`, `StatusTables`, `StatusTablesView` — all import `MapView` (2 via barrel `features/map/index.js`).
- **Settings page** (`pages/SettingsPage.jsx`): Has Profile, Password, Notifications, Appearance sections. No tabs — all sections stacked vertically. Uses existing CSS design system (`.settings-section`, `.setting-row`, `.toggle-switch`). No Advanced section yet.
- **Redux store** (`app/store/index.js`): `auth` + `gates` slices + RTK Query `api` slice. No `settings` or `ui` slice.
- **RTK Query API** (`app/store/api/api.js`): Has `updateUserDetails` mutation hitting `PUT /api/auth/user-change`. Could extend to include `mapStyle` field.
- **Dark mode**: Persisted via `localStorage.setItem('sensemante-dark', ...)`. No server persistence. Pattern exists for client-only prefs.
- **mapcn**: Requires Tailwind + shadcn + MapLibre GL. Young (pre-1.0, 1 maintainer). CARTO tiles need commercial license. NOT worth migrating to now.
- **Better approach**: Keep leaflet. Add free tile provider options (CartoDB Positron, CartoDB Dark Matter, OpenFreeMap, OSM). User picks style in settings. No library swap needed.

## Objectives
### Core Objective
Let users switch map tile style via Settings page. Keep leaflet as engine. No Tailwind/shadcn dependency.

### Deliverables
- [x] Tile provider registry config with 4+ providers (name, url, attribution, preview label)
- [x] `ui` Redux slice with `mapStyle` state + localStorage persistence
- [x] MapView reads `mapStyle` from Redux, renders correct `TileLayer`
- [x] Settings page "Advanced" section with map style picker UI (radio group)
- [x] API persistence: `mapStyle` sent with `updateUserDetails`, loaded on login
- [x] Default fallback to OSM when no preference set

### Definition of Done
- [x] User can pick map style in Settings → Advanced section
- [x] Map re-renders with chosen tiles across all 3 consumer locations
- [x] Preference survives page reload (localStorage) and re-login (API)
- [x] Invalid/missing preference falls back to OSM silently

### Guardrails (Must NOT)
- Do NOT add Tailwind, shadcn/ui, or mapcn as dependencies
- Do NOT replace leaflet with MapLibre (keep as future option, not now)
- Do NOT break existing Marker/Popup/divIcon behavior
- Do NOT add mapcn vector option in this iteration (future work)

## TODOs

- [x] 1. Create tile provider registry
  **What**: New file `features/map/config/tileProviders.js`. Export array of provider objects: `{ id, name, url, attribution, description }`. Providers: `osm` (default), `carto-positron`, `carto-dark`, `openfreemap`. Export helper `getProviderById(id)` that returns provider or falls back to OSM. IDs as string constants.
  **Files**: `server/frontend/src/features/map/config/tileProviders.js` (new)
  **Acceptance**: Import `getProviderById('carto-positron')` returns correct URL. `getProviderById('nonexistent')` returns OSM provider.

- [x] 2. Create `ui` Redux slice with `mapStyle`
  **What**: New slice `app/store/slices/uiSlice.js`. State: `{ mapStyle: 'osm' }`. Reducers: `setMapStyle(state, action)` — sets `mapStyle`, persists to localStorage key `sensemante-map-style`. Initialize `mapStyle` from localStorage in `initialState` (same pattern as dark mode in `SettingsPage.jsx`). Register in store at `store/index.js` as `ui: uiReducer`.
  **Files**: `server/frontend/src/app/store/slices/uiSlice.js` (new), `server/frontend/src/app/store/index.js` (modify — add import + reducer)
  **Acceptance**: `useAppSelector(state => state.ui.mapStyle)` returns current style. Dispatching `setMapStyle('carto-dark')` updates state + writes localStorage.

- [x] 3. Refactor MapView to use dynamic TileLayer
  **What**: Import `useAppSelector` and `getProviderById`. Read `state.ui.mapStyle` from Redux. Pass resolved provider's `url` and `attribution` to `<TileLayer>`. Keep all Marker/Popup/divIcon logic untouched. If `mapStyle` changes, leaflet's `TileLayer` re-renders with new URL (react-leaflet handles this on key change — add `key={mapStyle}` to `TileLayer` to force remount on style change).
  **Files**: `server/frontend/src/features/map/components/MapView.jsx` (modify)
  **Acceptance**: Change `mapStyle` in Redux devtools → TileLayer updates. Markers, popups, icons still work. OSM renders when `mapStyle` is `'osm'`. CartoDB Positron renders when `'carto-positron'`.

- [x] 4. Add "Advanced" section to Settings page with map style picker
  **What**: Add new `<div className="settings-section">` after "Appearance" section in `SettingsPage.jsx`. Title: "Advanced". Inside: map style picker as radio button group. Use `useAppSelector` to read current `mapStyle`, `useAppDispatch` + `setMapStyle` to update. Each option shows provider name + description. Style with existing CSS classes (`.setting-row`, `.setting-label`, `.setting-desc`). Add small preview description per provider.
  **Files**: `server/frontend/src/pages/SettingsPage.jsx` (modify)
  **Acceptance**: Settings page shows "Advanced" section with radio buttons. Clicking a radio updates Redux state + localStorage. Navigating to /map shows chosen tile style.

- [x] 5. Add CSS for map style radio group
  **What**: Add `.map-style-options` and `.map-style-option` CSS rules to `theme.css`. Radio options displayed as a vertical stack (not inline). Each option is a `.setting-row` with radio + label + description. Active option highlighted with left border (`.map-style-option.active` using `var(--blue-600)`). Follow existing `.setting-row` / `.filter-tab.active` patterns.
  **Files**: `server/frontend/src/shared/styles/theme.css` (modify — append to Settings section)
  **Acceptance**: Radio group renders cleanly in light + dark mode. Active option visually distinct.

- [x] 6. API persistence — send mapStyle with user details
  **What**: When user clicks "Save Changes" in settings, include `mapStyle` in the `updateUserDetails` payload. On login/user-details fetch, read `mapStyle` from user object and dispatch `setMapStyle` to sync Redux + localStorage. In `SettingsPage`, read `user.mapStyle` on mount — if present and different from localStorage, prefer server value. In `api.js`, the `getUserDetails` response likely needs a `mapStyle` field from the backend. For now: client-side localStorage is source of truth. Add a `useEffect` in `SettingsPage` that, when `user` loads, dispatches `setMapStyle(user.mapStyle)` if `user.mapStyle` exists. The `handleSave` already sends a payload to `updateUserDetails` — extend it with `mapStyle` field.
  **Files**: `server/frontend/src/pages/SettingsPage.jsx` (modify — `handleSave` + mount effect)
  **Acceptance**: `mapStyle` included in save payload. On page load with server data, Redux syncs to server preference if available.

- [x] 7. Update map attribution dynamically
  **What**: When tile provider changes, the `map-attribution` div in `MapPage.jsx` (bottom-right overlay) should reflect the active provider's attribution. Read `mapStyle` from Redux in `MapPage`, resolve provider via `getProviderById`, render `provider.attribution` text. Currently hard-coded as "Map data &copy; OpenStreetMap". Make it dynamic.
  **Files**: `server/frontend/src/pages/MapPage.jsx` (modify)
  **Acceptance**: Attribution text changes when map style changes. CartoDB styles show "&copy; OpenStreetMap contributors &copy; CARTO". OSM shows current text.

- [x] 8. Handle dark mode + map style interaction
  **What**: When dark mode is active, auto-suggest `carto-dark` tiles (but do not force — user can override). Add a visual hint in the Advanced section: if dark mode on and current map style is `osm` or `carto-positron`, show a subtle suggestion "Consider Dark Matter for dark mode". This is a non-blocking UI hint only.
  **Files**: `server/frontend/src/pages/SettingsPage.jsx` (modify — add hint text in Advanced section)
  **Acceptance**: In dark mode with light map style, hint appears. Dismissing or switching style removes hint. No forced switching.

## File Change Summary

| File | Action | Purpose |
|------|--------|---------|
| `features/map/config/tileProviders.js` | Create | Tile provider registry |
| `app/store/slices/uiSlice.js` | Create | `mapStyle` state + localStorage persistence |
| `app/store/index.js` | Modify | Register `ui` reducer |
| `features/map/components/MapView.jsx` | Modify | Dynamic TileLayer from Redux |
| `pages/SettingsPage.jsx` | Modify | Advanced section + style picker + API save |
| `shared/styles/theme.css` | Modify | Map style picker CSS |
| `pages/MapPage.jsx` | Modify | Dynamic attribution |

## Implementation Order

1 → 2 → 3 → 5 → 4 → 7 → 6 → 8

Rationale: Registry + state first (1, 2). Wire to MapView (3). Style the picker (5). Build picker UI (4). Attribution (7). API persistence (6). Dark mode hint (8).

## Verification
- [x] All 3 map consumers (MapPage, StatusTables, StatusTablesView) render with chosen style
- [x] Style preference persists across page navigation (Redux) and page reload (localStorage)
- [x] Settings page shows Advanced section with radio group
- [x] Default is OSM when no preference stored
- [x] Attribution updates per provider
- [x] Dark mode + map style hint works
- [x] `npm run build` succeeds (no import errors)
- [x] No regressions in marker rendering, popup content, or gate data display
