# Project Overview

## Context

The SenseMate frontend is a **floodgate monitoring and control dashboard** developed by the RIOT (Reliable IoT) team (PO-2026). It allows municipal operators to monitor and control IoT-connected flood gates across Hamburg.

## What the Frontend Does

| Capability | How |
|---|---|
| User authentication & session management | JWT in HttpOnly cookies + Redux (authSlice), CSRF protection |
| Real-time gate status visualization | Leaflet map + summary info boxes |
| Gate control (open/close/OOS) | REST API calls from UI dialogs |
| Downlink command dispatch | REST API with rate limiting (10-command cap) |
| Activity tracking | Chronological event log with real-time updates |
| Push notifications | STOMP over WebSocket |
| Guest access | Unauthenticated read-only dashboard |

## User Roles

| Role | Access Level | Route |
|---|---|---|
| **Controller** | Full CRUD, downlinks, gate management | `/dashboard` |
| **Viewer** | Read-only — gate status and map | `/dashboard` (conditional rendering in DashboardPage) |
| **Guest** | No auth — read-only, limited features | `/dashboard-guest` |

## Design Decisions

1. **Feature-based folder structure** — organized by domain (auth, gates, map, activities, notifications, shell) rather than by technical layer
2. **Redux Toolkit for auth, RTK Query for server state** — centralized auth in `authSlice`, API calls via RTK Query auto-generated hooks
3. **WebSocket for controllers, polling for viewers** — controllers get real-time STOMP updates; viewer and guest dashboards use 300ms polling
4. **WebSocket managed via Redux middleware** — `wsMiddleware.js` handles STOMP lifecycle, dispatches actions to Redux store
5. **Route-level guards** — `ProtectedRoute` (role-gated) and `PublicOnlyRoute` wrap routes in `App.jsx`, no inline auth checks needed
6. **Downlink rate limiting** — server-side 10-command counter with admin-password reset prevents excessive IoT commands
