# Authentication Flow

## Overview

Authentication uses **JWT tokens** stored in **HttpOnly cookies** set by the Spring Boot backend. The frontend uses **RTK Query** for all API calls and **Redux** (`authSlice`) for centralized auth state. Route-level guards (`ProtectedRoute`, `PublicOnlyRoute`) enforce access control.

## Flow Diagram

```mermaid
sequenceDiagram
    participant Browser
    participant Redux as Redux Store (authSlice)
    participant RTK as RTK Query (api.js)
    participant Backend as Spring Boot

    Note over Browser,Redux: App mounts
    Redux->>RTK: dispatch(initializeAuth())
    RTK->>Backend: GET /api/auth/user-details (credentials: include, X-XSRF-TOKEN)
    alt Valid session cookie
        Backend-->>RTK: User data + 200
        RTK-->>Redux: fulfilled → status: authenticated, user: {name, email, role, workerId}
    else No cookie or expired
        Backend-->>RTK: 401
        RTK-->>Redux: rejected → status: unauthenticated
    end

    Note over Browser,Backend: User logs in
    Browser->>RTK: useLoginMutation({email, password})
    RTK->>Backend: POST /api/auth/login
    Backend-->>Browser: Set-Cookie: jwt=<token>; HttpOnly; SameSite=Lax; Max-Age=36000
    Backend-->>RTK: UserDetailsResponse {name, email, role, workerId}
    RTK-->>Redux: dispatch(auth/setUser)
    Browser->>Browser: Navigate by role (controller → /dashboard, viewer → /dashboard)

    Note over Browser,Backend: User logs out
    Browser->>RTK: useLogoutMutation()
    RTK->>Backend: POST /api/auth/logout
    Backend-->>Browser: Set-Cookie: jwt=""; Max-Age=0
    RTK-->>Redux: dispatch(auth/clearAuth)
    Browser->>Browser: Navigate to /
```

## Session Persistence

1. **On login/register**: Backend sets an `HttpOnly` cookie (`jwt`, 10-hour expiry, `SameSite=Lax`, path=`/`). The response body contains a `UserDetailsResponse` which the frontend stores in Redux.

2. **On app load**: `App.jsx` dispatches `initializeAuth()`, which calls `GET /api/auth/user-details` via `fetchBaseQuery` (same CSRF + credentials setup as RTK Query). If the cookie is valid, the user is authenticated without re-entering credentials.

3. **On logout**: `useLogoutMutation` calls `POST /api/auth/logout`. On success, the backend clears the cookie and the frontend dispatches `auth/clearAuth`. On failure, auth state is preserved so the user can retry.

## Route Guards

### ProtectedRoute
Wraps routes that require authentication and optionally a specific role:
- While `auth.status === 'loading'` → shows `CircularProgress`
- If `unauthenticated` → redirects to `/login`
- If role doesn't match → redirects to `/`

### PublicOnlyRoute
Wraps routes that should only be visible to unauthenticated users (login, register):
- If `authenticated` → redirects to role-appropriate dashboard
- If `unauthenticated` → renders the child route

## Redux State (authSlice)

```javascript
{
  user: { name, email, role, workerId } | null,
  status: 'loading' | 'authenticated' | 'unauthenticated',
  error: string | null
}
```

Actions: `setUser`, `clearAuth`, `setAuthError`
Thunk: `initializeAuth` (called on app mount)

## RTK Query Auth Endpoints (api.js)

| Endpoint | Method | Description |
|----------|--------|-------------|
| `login` | POST `/api/auth/login` | Authenticates user, sets cookie |
| `register` | POST `/api/auth/register` | Creates account, sets cookie |
| `getUserDetails` | GET `/api/auth/user-details` | Returns current user (tag: `Auth`) |
| `updateUserDetails` | PUT `/api/auth/user-change` | Updates name/password |
| `logout` | POST `/api/auth/logout` | Clears session, expires cookie |

## CSRF Protection

CSRF tokens are handled by Spring's `CookieCsrfTokenRepository`:
- Backend sets a JS-readable `XSRF-TOKEN` cookie
- RTK Query's `prepareHeaders` reads it via `getCookie('XSRF-TOKEN')` and sets the `X-XSRF-TOKEN` header on every request
- `initializeAuth` uses the same `fetchBaseQuery` config, so CSRF is included on app init too

## Cookie Configuration

All cookies are set by the backend with:
- `HttpOnly=true` (JS cannot read `jwt` cookie)
- `SameSite=Lax`
- `Secure` = `${app.cookie.secure}` (true in production)
- `Path=/`

| Endpoint | Cookie | Max-Age |
|----------|--------|---------|
| `/auth/login` | `jwt=<token>` | 36000s (10h) |
| `/auth/register` | `jwt=<token>` | 36000s (10h) |
| `/auth/logout` | `jwt=""` | 0 (delete) |

## Security Considerations

1. **JWT cookie is `HttpOnly`** — not accessible via JavaScript, mitigating XSS token theft. The `XSRF-TOKEN` cookie is intentionally `HttpOnly=false` so the frontend can read it for CSRF headers.

2. **CSRF protection enabled** — double-submit cookie pattern via `X-XSRF-TOKEN` header on all mutating requests.

3. **In-memory token store** — the backend tracks issued tokens in a `ConcurrentHashMap`. Sessions are lost on server restart and this does not scale horizontally.

4. **No token refresh** — when the 10-hour JWT expires, the user must log in again.


