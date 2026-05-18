# Project Overview

## Context

The SenseMate frontend is a **floodgate monitoring and control dashboard** developed by the RIOT (Reliable IoT) team (PO-2026). It allows municipal operators to monitor and control IoT-connected flood gates across Hamburg.

## What the Frontend Does

| Capability | How |
|---|---|
| User authentication & session management | JWT in cookies + Axios headers, React Context |
| Real-time gate status visualization | Leaflet map + summary info boxes |
| Gate control (open/close/OOS) | REST API calls from UI dialogs |
| Downlink command dispatch | REST API with rate limiting (10-command cap) |
| Activity tracking | Chronological event log with real-time updates |
| Push notifications | STOMP over SockJS WebSocket |
| Guest access | Unauthenticated read-only dashboard |

## User Roles

| Role | Access Level | Route |
|---|---|---|
| **Controller** | Full CRUD, downlinks, gate management | `/dashboard` |
| **Viewer** | Read-only — gate status and map | `/dashboard-view` |
| **Guest** | No auth — read-only, limited features | `/dashboard-guest` |

## Design Decisions

1. **Feature-based folder structure** — organized by domain (auth, gates, map, activities, notifications, shell) rather than by technical layer
2. **React Context for auth, local state for everything else** — no Redux/Zustand; the app is flat enough that this suffices
3. **WebSocket for controllers, polling for viewers** — controllers get real-time STOMP updates; viewer and guest dashboards use 300ms polling
4. **Per-component WebSocket lifecycle** — each component manages its own SockJS/STOMP connection in `useEffect` with cleanup
5. **Imperative auth guards** — each protected page checks `isAuthenticated` on mount and shows `AlertDialogIllegal`, not a route wrapper HOC
6. **Downlink rate limiting** — server-side 10-command counter with admin-password reset prevents excessive IoT commands
