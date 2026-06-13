# Fix Redesign Gaps — E2E Test Failures

Branch: `76-frontend-redesign` at `dcc9f6a`
Goal: Fix all missing features and selector mismatches so 18/18 chromium e2e tests pass.

---

## Design Intent vs Reality

The redesign uses **1 dashboard route** (`/dashboard`) with **auth-based conditional rendering**:
- Controller → `StatusTables` (full CRUD)
- Viewer → `StatusTablesView` (read-only)

✅ **This IS implemented** in `DashboardPage.jsx` lines 10-16.

The **guest dashboard** is a different concept — unauthenticated users viewing public gate data. This is what's missing.

The **user page** is integrated into the app shell (sidebar bottom shows user, topbar shows avatar, `/settings` has profile). But there's no `/userpage` route and no way to log out.

---

## Tasks

- [x] 1/13. **S7** — Fix status mapping: add `"OPEN"` → "Open" in `statusInfo()` in both `StatusTables.jsx` and `StatusTablesView.jsx`
  - **Files**: `server/frontend/src/features/gates/components/StatusTables.jsx`, `server/frontend/src/features/gates/components/StatusTablesView.jsx`
  - **Acceptance**: `statusInfo("OPEN")` returns `{ cls: "status-open", label: "Open" }` in both files

- [x] 2/13. **S1, S2, S3** — Fix test selectors: table class, stat cards, search placeholder
  - **Files**: `server/frontend/tests/dashboard.spec.ts`, `server/frontend/tests/auth.spec.ts`
  - **Acceptance**: All `table.status-table` → `table.gate-table`, `.info-boxes .box h1` → `.stat-card .stat-number`, `Search gates...` → `Search gates…`

- [x] 3/13. **F2** — Update test: viewer redirect expects `/dashboard` not `/dashboard-view`
  - **Files**: `server/frontend/tests/auth.spec.ts`, `server/frontend/tests/utils.ts`
  - **Acceptance**: Test expects viewer to land on URL matching `/dashboard` (not `/dashboard-view`)

- [x] 4/13. **F1** — Add `/dashboard-guest` route + `DashboardGuestPage` component
  - **Files**: `server/frontend/src/app/App.jsx`, new file `server/frontend/src/pages/DashboardGuestPage.jsx`
  - **Acceptance**: `/dashboard-guest` route exists, renders `StatusTablesView` with minimal header, no `AppLayout` shell, no auth required, shows gate data, no "Create Gate" button

- [x] 5/13. **F4 + F3** — Add `/userpage` route + mount `LogoutButton`
  - **Files**: `server/frontend/src/app/App.jsx`, `server/frontend/src/pages/SettingsPage.jsx`, `server/frontend/src/features/shell/components/Sidebar.jsx`
  - **Acceptance**: `/userpage` route exists (can be alias for settings or standalone page), `LogoutButton` is mounted and visible, clicking logout redirects to `/`

- [x] 6/13. **F5** — Add `id="notificationButton"` + wire `NotificationPopup` toggle
  - **Files**: `server/frontend/src/features/shell/components/Topbar.jsx`, `server/frontend/src/features/shell/components/AppLayout.jsx`
  - **Acceptance**: Notification button has `id="notificationButton"`, clicking toggles `NotificationPopup` visibility, popup shows "Notifications" heading and notification messages

- [x] 7/13. **S4** — Fix Map/List view tab roles and names
  - **Files**: `server/frontend/src/features/gates/components/StatusTables.jsx`, `server/frontend/src/features/gates/components/StatusTablesView.jsx`
  - **Acceptance**: View toggle buttons have `role="tab"`, `aria-selected` attribute, and names "Map View"/"List View" (matching test expectations)

- [x] 8/13. **S5** — Fix expand row button: change `<a>` to `<button>` with `aria-label="expand row"`
  - **Files**: `server/frontend/src/features/gates/components/StatusTables.jsx`, `server/frontend/src/features/gates/components/StatusTablesView.jsx`
  - **Acceptance**: Expand activator is a `<button>` (not `<a>`) with `aria-label="expand row"`, test `getByRole('button', { name: 'expand row' })` finds it

- [x] 9/13. **S6** — Fix create gate form label-input linkage
  - **Files**: `server/frontend/src/features/gates/components/StatusTables.jsx`
  - **Acceptance**: All form fields in create gate dialog have `id` attributes, labels have matching `htmlFor`, `getByLabel('Location')` etc. work in Playwright

- [x] 10/13. **F6** — Add "Our Mission" heading/section to `LandingPage.jsx`
  - **Files**: `server/frontend/src/pages/LandingPage.jsx`
  - **Acceptance**: Landing page contains visible text "Our Mission" (test `page.getByText('Our Mission')` passes)

- [x] 11/13. **F7** — Wire `ProtectedRoute` around auth-required routes
  - **Files**: `server/frontend/src/app/App.jsx`
  - **Acceptance**: Dashboard, settings, userpage routes wrapped with `ProtectedRoute`, unauthenticated users redirected to `/login`

- [x] 12/13. **A1** — Verify and fix API test for login token response
  - **Files**: `server/frontend/tests/api.spec.ts`
  - **Acceptance**: API test for login verifies token presence correctly (either in body or via cookie)

- [x] 13/13. Run full e2e test suite and verify 18/18 chromium tests pass
  - **Files**: none (verification only)
  - **Acceptance**: `npx playwright test --project=chromium` reports 18 passed, 0 failed

---

## Reference: Detailed Gap Analysis

### Missing Features

- **F1**: Guest Dashboard — `/dashboard-guest` route MISSING
- **F2**: Viewer redirect — test expects `/dashboard-view`, redesign uses `/dashboard`
- **F3**: Logout Button — component exists, NOT MOUNTED
- **F4**: User Page — `/userpage` route MISSING
- **F5**: Notification Button — no `id`, no toggle wired
- **F6**: "Our Mission" text MISSING from Landing Page
- **F7**: ProtectedRoute — NOT WIRED

### Selector Mismatches

- **S1**: Table CSS class: `status-table` → `gate-table`
- **S2**: Info boxes: `.info-boxes .box h1` → `.stat-card .stat-number`
- **S3**: Search placeholder: `...` → `…`
- **S4**: Map/List View: role `tab` + "Map View"/"List View" → `<button>` + "Map"/"List"
- **S5**: Expand row: `button { name: 'expand row' }` → `<a>` "▶ Activities"
- **S6**: Create Gate form: no label-input linkage
- **S7**: Status value: `"OPEN"` maps to "Out of Service"

### API Shape

- **A1**: Login response: `.token` in body
