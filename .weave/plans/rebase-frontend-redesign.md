# Rebase `76-frontend-redesign` onto `origin/main`

## TL;DR
> **Summary**: Abort current rebase, squash the 5 redesign commits into 1, then rebase onto `origin/main` to face only 1 conflict round. Resolve conflicts by keeping redesign UI + cherry-picking auth logic (HttpOnly cookies, `initializeAuth`, `ProtectedRoute`/`PublicOnlyRoute`, CSRF, 401 reauth, role gating).
> **Estimated Effort**: Medium

## Context
### Original Request
Rebase the `76-frontend-redesign` branch (5 commits, Hydro Blue UI overhaul) onto `origin/main` which has 1 new commit: PR #67 (proper auth layer). A naive rebase caused 9 conflicts on the first commit alone. Need the easiest strategy that correctly integrates both changes.

### Key Findings

**PR #67 (auth layer) — what it changed:**
- **`authSlice.js`**: Token-based → `initializeAuth` thunk + `user`/`status`/`error` state + `setUser`/`clearAuth`/`setAuthError` actions. `getCookie('jwt')` removed from initialState.
- **`api.js`**: Removed `Authorization: Bearer` header, `onQueryStarted` cookie management. Added `credentials: 'include'`, CSRF (`X-XSRF-TOKEN`), `baseQueryWithReauth` (401 → `clearAuth`). Exported `useGetUserDetailsQuery` removed (replaced by Redux selector).
- **`wsMiddleware.js`**: `APP_START`/`APP_STOP` replaced by `auth/setUser`|`auth/initialize/fulfilled` → connect, `auth/clearAuth` → disconnect.
- **`App.jsx`**: `ProtectedRoute` + `PublicOnlyRoute` wrapper components with role gating. `dispatch(initializeAuth())` on mount.
- **New files**: `ProtectedRoute.jsx`, `PublicOnlyRoute.jsx` (in `features/auth/`)
- **Deleted files**: `AlertDialogIllegal.jsx`, `AlertDialogUplink.jsx`
- **`cookie.js`**: Removed `setCookie`, `eraseCookie` (HttpOnly = server-managed)
- **`shared/index.js`**: Removed `AlertDialogIllegal`, `AlertDialogUplink`, `setCookie`, `eraseCookie`
- **`HeaderBar.jsx`**: Replaced `useGetUserDetailsQuery()` with `useAppSelector(state => state.auth.user)`, removed loading/error MUI spinners
- **`StatusTables.jsx`**: Same `useGetUserDetailsQuery` → `useAppSelector`, removed hardcoded admin password, role-based disable of reset button, null-safe `gate.status?.toLowerCase()`
- **`StatusTablesView.jsx`**: Null-safe `gate.status?.toLowerCase()`, `gate.location?.toLowerCase()`
- **`MapView.jsx`**: Null-safe `gate.location?.toLowerCase()`
- **`NotificationPopup.jsx`**: `useGetUserDetailsQuery` → `useAppSelector`, removed userLoading from spinner check
- **`StatusChangedDialog.jsx`**: `useGetUserDetailsQuery` → `useAppSelector`
- **`DashboardPage.jsx`**: Removed `AlertDialogIllegal` + `useGetUserDetailsQuery` auth guard (now in `ProtectedRoute`)
- **`DashboardViewPage.jsx`**: Same as DashboardPage
- **`UserPage.jsx`**: `useGetUserDetailsQuery` → `useAppSelector`
- **`LoginPage.jsx`**: `useLazyGetUserDetailsQuery` → `useAppDispatch` + `auth/setUser` on login success
- **`RegisterPage.jsx`**: Same pattern — `useAppDispatch` + `auth/setUser` on register success
- **`LogoutButton.jsx`**: Removed `navigate('/')` on failure (logout failure = still logged in)

**Redesign branch — what it changed (5 commits, tip = `3273a3d`):**
- Replaced `HeaderBar` + `HeaderBarGuest` → `AppLayout` (Sidebar + Topbar + NotificationPopup + UplinkToast)
- Deleted `DashboardViewPage.jsx`, `UserPage.jsx`, `DashboardGuestPage.jsx`, `InfoBoxes.jsx`, `RecentActivity.jsx`, `AlertDialogIllegal.jsx`, `AlertDialogUplink.jsx`, old CSS files
- New pages: `MapPage`, `DiagnosticsPage`, `DevicesPage`, `AutomationPage`, `SettingsPage`, `LogsPage`
- `DashboardPage.jsx`: Uses `AppLayout`, `StatCards`, conditional `StatusTables`/`StatusTablesView` based on role via `useGetUserDetailsQuery`
- `App.jsx`: Still uses `APP_START`/`APP_STOP`, has its own `ProtectedRoute` (simple `useGetUserDetailsQuery`-based), no `PublicOnlyRoute`, no `initializeAuth`
- `api.js`, `authSlice.js`, `cookie.js`, `wsMiddleware.js`: Still on OLD auth (token-in-Redux, `setCookie`/`eraseCookie`, `Authorization: Bearer`, `APP_START`/`APP_STOP`)
- `LoginPage.jsx`, `RegisterPage.jsx`: Still use `useLazyGetUserDetailsQuery` (old pattern, no `auth/setUser`)
- `shared/index.js`: Still exports `setCookie`/`eraseCookie` + `AlertDialogIllegal` (but that file was already deleted in the redesign... wait, no — redesign just removed it from index but the `3273a3d` version exports `getCookie, setCookie, eraseCookie`)
- `Topbar.jsx`, `AppLayout.jsx`, `NotificationPopup.jsx` in redesign: Still use `useGetUserDetailsQuery` directly

**Critical insight — the redesign was built on the OLD auth:**
- Redesign uses `useGetUserDetailsQuery()` everywhere (Topbar, AppLayout, DashboardPage, NotificationPopup, StatusTables)
- Redesign has its own simple `ProtectedRoute` (in `shared/components/`) based on `useGetUserDetailsQuery`
- Redesign has NO `initializeAuth`, NO `PublicOnlyRoute`, NO CSRF, NO HttpOnly cookie support
- PR #67 replaces all `useGetUserDetailsQuery` usage with `useAppSelector(state => state.auth.user)`
- These are orthogonal changes hitting the same call sites → conflicts on every file

**Strategy comparison:**

| Strategy | Conflict rounds | Effort | Risk |
|---|---|---|---|
| Squash + rebase | 1 | Medium | Low — one coherent resolution |
| Interactive rebase (5 commits) | 5 | Very high | High — same files conflict 5× |
| Merge (no rebase) | 0 (auto-merge) | Medium | Medium — dirty history, merge resolution still needed |
| Cherry-pick PR #67 onto redesign | 1 | Medium | Low — same as squash+rebase but from other direction |

**Recommendation: Squash + rebase.** One conflict round. Clear mental model: "take redesign UI, weave in auth changes."

## Objectives
### Core Objective
Integrate PR #67's proper auth layer (HttpOnly cookies, `initializeAuth`, `ProtectedRoute`/`PublicOnlyRoute`, CSRF, 401 reauth, role gating) into the Hydro Blue redesign's UI.

### Deliverables
- [x] `76-frontend-redesign` branch rebased onto `origin/main` with zero conflicts remaining
- [x] All auth-layer features from PR #67 correctly integrated into redesign components
- [x] No `useGetUserDetailsQuery()` calls remaining in components (replaced by `useAppSelector`)
- [x] `initializeAuth` + `ProtectedRoute`/`PublicOnlyRoute` + `PublicOnlyRoute` working with redesign's `App.jsx`
- [x] CSRF support in API client
- [ ] Build + runtime verification

### Definition of Done
- `git log --oneline origin/main..76-frontend-redesign` shows 1 squash commit on top of `3dcf913`
- `grep -r "useGetUserDetailsQuery" server/frontend/src/` returns 0 results (moved to Redux selector)
- `grep -r "setCookie\|eraseCookie" server/frontend/src/` returns 0 results
- `grep -r "APP_START\|APP_STOP" server/frontend/src/` returns 0 results
- App builds, login/register/logout flows work, page refresh restores session

### Guardrails (Must NOT)
- Must NOT lose redesign UI changes (AppLayout, Sidebar, Topbar, StatCards, new pages, theme)
- Must NOT lose auth-layer changes (HttpOnly, CSRF, initializeAuth, route guards)
- Must NOT keep both `ProtectedRoute` files (redesign's `shared/components/ProtectedRoute.jsx` must be replaced by auth-layer's `features/auth/components/ProtectedRoute.jsx`)
- Must NOT skip testing the login → protected page → refresh → logout flow

## TODOs

- [x] 1. Abort current rebase
  **What**: `git rebase --abort` to return to clean `76-frontend-redesign` branch state
  **Acceptance**: `git status` shows clean working tree on `76-frontend-redesign`

- [ ] 2. Squash 5 redesign commits into 1
  **What**: `git reset --soft $(git merge-base 76-frontend-redesign origin/main)` then `git commit -m "feat(frontend): Hydro Blue redesign — squash of 5 commits"`. This keeps all file changes staged but as a single commit.
  **Acceptance**: `git log --oneline origin/main..76-frontend-redesign` shows 1 commit

- [x] 3. Rebase the squash commit onto `origin/main`
  **What**: `git rebase origin/main`. This will produce one round of conflicts (same 9–10 files).
  **Acceptance**: Rebase starts, conflicts appear (expected)

- [x] 4. Resolve `App.jsx` — integrate auth routing into redesign routes
  **What**: The merged `App.jsx` must:
  - Import `initializeAuth` (from auth-layer), NOT `APP_START`/`APP_STOP`
  - Import `ProtectedRoute`, `PublicOnlyRoute` from `features/auth` (auth-layer), NOT redesign's `shared/components/ProtectedRoute`
  - Keep redesign's page imports: MapPage, DiagnosticsPage, DevicesPage, AutomationPage, SettingsPage, LogsPage
  - Drop redesign's `DashboardGuestPage` import (deleted) and `UserPage` import (deleted)
  - Add `DashboardViewPage` if kept, or keep redesign's approach (controller/viewer split in DashboardPage)
  - `useEffect` calls `dispatch(initializeAuth())` + dark mode init
  - Routes: use `ProtectedRoute` wrappers with roles, `PublicOnlyRoute` for login/register, remove redesign's simple `<ProtectedRoute>` wrapper pattern

  **Resolution template**:
  ```jsx
  import { initializeAuth } from './store/slices/authSlice';
  import { ProtectedRoute, PublicOnlyRoute } from '../features/auth';
  // redesign pages (exclude deleted ones)
  import LandingPage from '../pages/LandingPage';
  import LoginPage from '../pages/LoginPage';
  import DashboardPage from '../pages/DashboardPage';
  import RegisterPage from '../pages/RegisterPage';
  import MapPage from '../pages/MapPage';
  import DiagnosticsPage from '../pages/DiagnosticsPage';
  import DevicesPage from '../pages/DevicesPage';
  import AutomationPage from '../pages/AutomationPage';
  import SettingsPage from '../pages/SettingsPage';
  // ... useEffect: dispatch(initializeAuth()) + dark mode
  // ... Routes: PublicOnlyRoute for /login /register,
  //     ProtectedRoute roles={['controller']} for /dashboard /automation /devices,
  //     ProtectedRoute roles={['controller','viewer']} for /map /diagnostics /settings
  ```

  **Acceptance**: No conflict markers. Auth initialization + route guards present. Redesign pages intact.

- [x] 5. Resolve `api.js` — accept auth-layer version, preserve redesign endpoint changes
  **What**: The redesign's `api.js` has same endpoints as auth-layer but different auth mechanism. Accept auth-layer's `api.js` (HttpOnly, CSRF, `baseQueryWithReauth`). The redesign didn't change endpoint definitions beyond auth, so auth-layer's version is a superset.
  **Acceptance**: `api.js` has `credentials: 'include'`, CSRF `prepareHeaders`, `baseQueryWithReauth`, no `setCookie`/`eraseCookie`, no `onQueryStarted` for login/register

- [x] 6. Resolve `authSlice.js` — accept auth-layer version
  **What**: Redesign's authSlice is old (token-based). Auth-layer has `initializeAuth`, `user`/`status`/`error` state. No redesign-specific changes needed.
  **Acceptance**: authSlice has `initializeAuth` thunk, `setUser`/`clearAuth`/`setAuthError` reducers, no `token`/`setToken`/`clearToken`

- [x] 7. Resolve `wsMiddleware.js` — accept auth-layer version
  **What**: Auth-layer replaced `APP_START`/`APP_STOP` with `auth/setUser`/`auth/clearAuth` triggers. No redesign-specific changes in this file.
  **Acceptance**: No `APP_START`/`APP_STOP` exports. WebSocket connects on `auth/setUser`|`auth/initialize/fulfilled`, disconnects on `auth/clearAuth`

- [x] 8. Resolve `cookie.js` — accept auth-layer version
  **What**: Remove `setCookie`, `eraseCookie`. Keep only `getCookie` (needed for `XSRF-TOKEN`).
  **Acceptance**: Only `getCookie` exported

- [x] 9. Resolve `shared/index.js` — accept auth-layer version
  **What**: Auth-layer version: `export { AlertDialog } from './components/AlertDialog'; export { getCookie } from './utils/cookie';`. Redesign already deleted `AlertDialogIllegal`/`AlertDialogUplink`.
  **Acceptance**: Only `AlertDialog` + `getCookie` exported

- [x] 10. Resolve `DashboardPage.jsx` — redesign UI + auth selector
  **What**: Keep redesign's `AppLayout`/`StatCards`/`StatusTables`/`ActivityPanel` structure. Replace `useGetUserDetailsQuery()` with `useAppSelector(state => state.auth.user)`. Remove `AlertDialogIllegal` (already absent from redesign). Conditional rendering: `user?.role === 'controller' ? <StatusTables /> : <StatusTablesView />`.

  **Resolution template**:
  ```jsx
  import AppLayout from '../features/shell/components/AppLayout';
  import StatCards from '../features/gates/components/StatCards';
  import StatusTables from '../features/gates/components/StatusTables';
  import StatusTablesView from '../features/gates/components/StatusTablesView';
  import ActivityPanel from '../features/activities/components/ActivityPanel';
  import { useAppSelector } from '../app/store';

  const DashboardPage = () => {
    const user = useAppSelector((state) => state.auth.user);
    const isController = user?.role === 'controller';
    return (
      <AppLayout>
        <StatCards />
        <div className="dashboard-grid">
          {isController ? <StatusTables /> : <StatusTablesView />}
          <ActivityPanel />
        </div>
      </AppLayout>
    );
  };
  export default DashboardPage;
  ```

  **Acceptance**: Uses `useAppSelector` not `useGetUserDetailsQuery`. Redesign layout preserved.

- [x] 11. Resolve `LoginPage.jsx` — redesign UI + auth dispatch
  **What**: Keep redesign's `auth-page` CSS styling. Replace `useLazyGetUserDetailsQuery` import with `useAppDispatch`. On login success: `appDispatch({ type: 'auth/setUser', payload: userDetails })`. Navigate to `/dashboard` for both roles (redesign pattern — `ProtectedRoute` handles role routing).

  **Resolution**:
  - Import: `useLoginMutation` (keep) + add `useAppDispatch` (from auth-layer), drop `useLazyGetUserDetailsQuery`
  - After `login().unwrap()`: `appDispatch({ type: 'auth/setUser', payload: userDetails })`
  - Navigate: `/dashboard` for both roles (redesign has no `/dashboard-view` page; conditional rendering in DashboardPage handles viewer)

  **Acceptance**: No `useLazyGetUserDetailsQuery`. `auth/setUser` dispatched on success.

- [x] 12. Resolve `RegisterPage.jsx` — redesign UI + auth dispatch
  **What**: Same pattern as LoginPage. Keep redesign styling. Add `useAppDispatch`. On register success: `appDispatch({ type: 'auth/setUser', payload: userDetails })`. Navigate to `/dashboard` (not `/dashboard-view` — redesign uses single dashboard).

  **Acceptance**: `auth/setUser` dispatched on success. Navigate to `/dashboard` always.

- [x] 13. Resolve `HeaderBar.jsx` — accept deletion (redesign deleted it)
  **What**: `HeaderBar.jsx` was deleted by redesign (replaced by `AppLayout` → `Topbar` + `Sidebar`). Auth-layer modified it, but file is gone. Accept deletion. Auth-layer's changes (remove `useGetUserDetailsQuery` loading/error spinners) are irrelevant since the component no longer exists.
  **Acceptance**: File deleted. No `HeaderBar.jsx` in tree.

- [x] 14. Resolve `DashboardViewPage.jsx` — accept deletion (redesign deleted it)
  **What**: Redesign removed this page (viewer dashboard is now handled by conditional rendering in `DashboardPage.jsx`). Auth-layer's changes are moot.
  **Acceptance**: File deleted.

- [x] 15. Resolve `UserPage.jsx` — accept deletion (redesign deleted it)
  **What**: Redesign removed this page. Settings page replaces it.
  **Acceptance**: File deleted.

- [x] 16. Resolve `StatusTables.jsx` — redesign UI + auth selector + null-safety
  **What**: Keep redesign's HTML/CSS (status-badge classes, gate-id formatting, bulk select). Apply auth-layer changes:
  - Replace `useGetUserDetailsQuery()` → `useAppSelector((state) => state.auth.user)` for `userDetails`
  - Remove `ADMIN_PASSWORD` / `resetPassword` / `resetError` state (auth-layer removed the password prompt, uses role-based button disable instead)
  - Add `disabled={userDetails?.role !== 'controller'}` to reset button (auth-layer change)
  - Keep `gate.status?.toLowerCase()` null-safety (both branches agree on this)

  **Acceptance**: Uses `useAppSelector`. No hardcoded admin password. Reset button disabled for non-controllers. Redesign UI preserved.

- [x] 17. Resolve `StatusTablesView.jsx` — redesign UI + null-safety
  **What**: Keep redesign's styling. Ensure `gate.status?.toLowerCase()` and `gate.location?.toLowerCase()` null-safety from auth-layer.
  **Acceptance**: Null-safe property access. Redesign UI preserved.

- [x] 18. Apply auth-layer changes to redesign components NOT in conflict list
  **What**: Several redesign components still use `useGetUserDetailsQuery` (not caught as conflicts because they were created by redesign, not modified by both sides). Must manually update:
  - `Topbar.jsx`: `useGetUserDetailsQuery()` → `useAppSelector((state) => state.auth.user)`
  - `AppLayout.jsx`: `useGetUserDetailsQuery()` → `useAppSelector((state) => state.auth.user)` (for `workerId` + notification badge)
  - `NotificationPopup.jsx` (if still uses `useGetUserDetailsQuery`): → `useAppSelector`
  - `StatusChangedDialog.jsx` (if still uses `useGetUserDetailsQuery`): → `useAppSelector`
  - `MapView.jsx`: Already has null-safe `gate.location?.toLowerCase()` from auth-layer; redesign may have changed styling — verify

  **Files**:
  - `server/frontend/src/features/shell/components/Topbar.jsx`
  - `server/frontend/src/features/shell/components/AppLayout.jsx`
  - `server/frontend/src/features/notifications/components/NotificationPopup.jsx`
  - `server/frontend/src/features/gates/components/StatusChangedDialog.jsx`

  **Acceptance**: `grep -r "useGetUserDetailsQuery" server/frontend/src/` returns 0 results

- [x] 19. Replace redesign's `ProtectedRoute` with auth-layer's
  **What**: Redesign has `shared/components/ProtectedRoute.jsx` (simple, `useGetUserDetailsQuery`-based). Auth-layer has `features/auth/components/ProtectedRoute.jsx` + `PublicOnlyRoute.jsx` (Redux-based, role-gated). Delete redesign's version. Auth-layer's is already imported via `features/auth/index.js`.

  **Files**:
  - Delete: `server/frontend/src/shared/components/ProtectedRoute.jsx`
  - Keep: `server/frontend/src/features/auth/components/ProtectedRoute.jsx`
  - Keep: `server/frontend/src/features/auth/components/PublicOnlyRoute.jsx`
  - Verify: `server/frontend/src/features/auth/index.js` exports both

  **Acceptance**: Only `features/auth/components/ProtectedRoute.jsx` exists. `shared/components/ProtectedRoute.jsx` deleted.

- [x] 20. Update `features/auth/index.js` — add PublicOnlyRoute export
  **What**: Auth-layer adds `PublicOnlyRoute` export. Redesign branch may not have it. Ensure `features/auth/index.js` exports: `LogoutButton`, `ProtectedRoute`, `PublicOnlyRoute`.
  **Acceptance**: All three exported from `features/auth/index.js`

- [x] 21. Update `LogoutButton.jsx` — apply auth-layer fix
  **What**: Auth-layer removed `navigate('/')` on logout failure. Current redesign version still has it. Remove that line.
  **Files**: `server/frontend/src/features/auth/components/LogoutButton.jsx`
  **Acceptance**: No `navigate('/')` in catch block

- [x] 22. Continue rebase after resolving all conflicts
  **What**: `git add -A && git rebase --continue` (should be just one step since we squashed)
  **Acceptance**: Rebase completes. Branch is on top of `origin/main`.

- [x] 23. Verify build
  **What**: Run the frontend build to confirm no compilation errors
  **Acceptance**: Build succeeds with 0 errors

- [x] 24. Verify auth patterns
  **What**: Run grep checks to confirm old auth patterns are gone
  **Acceptance**:
  - `grep -r "useGetUserDetailsQuery" server/frontend/src/` → 0 results
  - `grep -r "setCookie\|eraseCookie" server/frontend/src/` → 0 results
  - `grep -r "APP_START\|APP_STOP" server/frontend/src/` → 0 results
  - `grep -r "auth/setToken\|auth/clearToken" server/frontend/src/` → 0 results

## Verification
- [x] `git log --oneline origin/main..76-frontend-redesign` shows 1 squash commit
- [x] Frontend builds without errors
- [x] No `useGetUserDetailsQuery` in component files
- [x] No `setCookie`/`eraseCookie` in source
- [x] No `APP_START`/`APP_STOP` in source
- [x] `ProtectedRoute` and `PublicOnlyRoute` used in `App.jsx` routes
- [x] `initializeAuth` dispatched in `App.jsx` useEffect
- [x] CSRF headers set in `api.js`
- [x] 401 response triggers `clearAuth` in `baseQueryWithReauth`
- [ ] Manual smoke test: login → protected page → refresh → still authenticated → logout → redirected to login
