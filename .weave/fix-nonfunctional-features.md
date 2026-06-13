# Plan: Fix Non-Functional Frontend Features

## Context
Post-redesign audit found 13 non-functional frontend features. This plan addresses user-approved fixes.

---

- [x] Task 1: Remove 4 no-op notification toggles from SettingsPage
- **What**: Remove `emailAlerts`, `pushNotifications`, `dailyDigest`, `criticalOnly` useState + JSX from SettingsPage.jsx. Remove the entire "Notification Preferences" section.
- **Files**: `server/frontend/src/pages/SettingsPage.jsx`
- **Acceptance**: No notification toggle state vars, no "Notification Preferences" section in JSX. Page still renders correctly (Profile, Change Password, Appearance, Save).

- [x] Task 2: Fix password change flow
- **What**: Fix `handleSave` to send `{ password: currentPassword, newPassword: newPassword }` to the backend (backend requires `password` = current for verification, `newPassword` = new). Show success feedback after save. Disable Save button when no changes are made. Show error feedback on failure.
- **Files**: `server/frontend/src/pages/SettingsPage.jsx`
- **Details**:
  - Fix payload: `{ password: currentPassword, newPassword: newPassword }`
  - Add success state/feedback (e.g. "Saved!" text or toast)
  - Disable Save button when `!newPassword && !confirmPassword` (no changes)
  - Replace empty `catch {}` with error state shown to user
- **Acceptance**: Password change works end-to-end. Save disabled when no changes. Success and error states visible to user.

- [x] Task 3: Make email field display-only (disable, not just readOnly)
- **What**: Add `disabled` attribute + visual treatment to the email input so it's clearly not editable (not just `readOnly` which looks like it could be edited).
- **Files**: `server/frontend/src/pages/SettingsPage.jsx`
- **Acceptance**: Email field clearly disabled/dimmed, user understands it's not editable.

- [x] Task 4: Wire MapPage status filters to MapView
- **What**: Pass `filters` state from MapPage to MapView as `statusFilter` prop. Update MapView to accept object `{ Closed: bool, Open: bool, OOS: bool }` and filter gates by mapping `gate.status` to checkbox key (OPEN→Open, CLOSED→Closed, OUT_OF_SERVICE→OOS). Also add search input to MapPage and wire to MapView's `search` prop.
- **Files**: `server/frontend/src/pages/MapPage.jsx`, `server/frontend/src/features/map/components/MapView.jsx`
- **Acceptance**: Toggling filter checkboxes in MapPage filters markers on map. Search input filters by id/location. Both work simultaneously.

- [x] Task 5: Fix OPENED → OPEN status mismatch (all frontend)
- **What**: Replace all `'OPENED'` status references with `'OPEN'` in frontend code. The backend API returns `'OPEN'` (not `'OPENED'`). OPENED exists in Java enum but is never produced by `fromCode()` or DB seeds.
- **Files**:
  - `server/frontend/src/pages/MapPage.jsx` (line 26)
  - `server/frontend/src/features/map/components/MapView.jsx` (lines 8, 43)
  - `server/frontend/src/features/gates/components/StatusTablesView.jsx` (lines 10, 36)
  - `server/frontend/src/features/gates/components/StatusTables.jsx` (lines 41, 78, 235, 663)
  - `server/frontend/src/features/gates/components/StatusChangedDialog.jsx` (line 35)
- **Details**: Replace `"OPENED"` with `"OPEN"` everywhere. Remove duplicate `case "OPEN":` entries that become redundant. Change `<option value="OPENED">OPENED</option>` to `<option value="OPEN">OPEN</option>`. StatCards already uses `'OPEN'` — no change needed there.
- **Acceptance**: All frontend status comparisons use `'OPEN'`. No `'OPENED'` strings remain in frontend. Gate status display, filtering, and stat counts all work correctly.

- [x] Task 6: Remove dead code — AlertDialog, AlertDialogIllegal, unused API hooks
- **What**:
  - Delete `server/frontend/src/shared/components/AlertDialog.jsx`
  - Delete `server/frontend/src/shared/components/AlertDialogIllegal.jsx`
  - Remove their re-exports from `server/frontend/src/shared/index.js`
  - Remove `addActivities` endpoint + `useAddActivitiesMutation` export from `api.js`
  - Remove `updateGate` endpoint + `useUpdateGateMutation` export from `api.js`
  - Remove `getNotifications` endpoint + `useGetNotificationsQuery` export from `api.js`
  - Keep cookie utils exports in `shared/index.js` (they are actively used)
- **Files**:
  - `server/frontend/src/shared/components/AlertDialog.jsx` (delete)
  - `server/frontend/src/shared/components/AlertDialogIllegal.jsx` (delete)
  - `server/frontend/src/shared/index.js`
  - `server/frontend/src/app/store/api/api.js`
- **Acceptance**: Files deleted, dead exports removed, no dangling imports. App still builds and runs.

- [x] Task 7: Restore uplink notifications
- **What**: Re-implement the uplink notification feature that was lost during redesign. When a WebSocket uplink arrives on `/topic/uplinks`, show a toast notification to the user. The `uplinkString` state + `uplinkReceived` action already exist in gatesSlice. The WS subscription already dispatches.
- **Implementation approach**:
  - Create a lightweight toast component (no MUI Snackbar — use CSS, matching the Hydro Blue design system)
  - Create `server/frontend/src/shared/components/UplinkToast.jsx` — reads `state.gates.uplinkString`, shows toast, auto-dismisses after ~5s, dispatches `resetUplinkString` on dismiss
  - Render `<UplinkToast />` in `AppLayout.jsx` (top-level, always visible)
  - Keep existing `uplinkReceived`/`resetUplinkString` actions in gatesSlice
  - Keep existing WS subscription in wsMiddleware
- **Files**:
  - `server/frontend/src/shared/components/UplinkToast.jsx` (new)
  - `server/frontend/src/features/shell/components/AppLayout.jsx`
  - `server/frontend/src/app/store/slices/gatesSlice.js` (may need `useAppSelector` export, likely already available)
- **Acceptance**: When an IoT uplink arrives, a toast notification appears showing the uplink message. Toast auto-dismisses. Multiple uplinks show in sequence (latest replaces previous, matching single `uplinkString` state). No console errors.

- [x] Task 8: Remove orphan UserPage
- **What**: Delete `UserPage.jsx` and remove its route from `App.jsx`. It's a dead-end page with no sidebar entry, no edit capability, and is fully superseded by SettingsPage.
- **Files**:
  - `server/frontend/src/pages/UserPage.jsx` (delete)
  - `server/frontend/src/app/App.jsx` (remove import + route)
- **Acceptance**: UserPage deleted, route removed, app still works, navigating to `/userpage` shows 404 or redirect.

---

## Dependency graph

```
Task 1 (remove toggles) → Task 2 (fix password) → Task 3 (disable email)
       (all modify SettingsPage.jsx — sequential)

Task 4 (MapPage filters) — independent
Task 5 (OPENED→OPEN) — independent but should run before Task 4 to avoid conflicts in MapPage/MapView
Task 6 (dead code) — independent
Task 7 (uplink toast) — independent
Task 8 (remove UserPage) — independent

Suggested batch order:
  Batch 1: Task 1, Task 5, Task 6, Task 7, Task 8 (parallel — no file overlaps)
  Batch 2: Task 2 (after Task 1 — same file)
  Batch 3: Task 3 (after Task 2 — same file)
  Batch 4: Task 4 (after Task 5 — shared files MapPage/MapView)
```
