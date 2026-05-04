# Project Overview

## Context

**SenseMate** is the frontend dashboard for a **flood disaster management system** developed by the RIOT (Reliable IoT) team as part of a university project (PO-2026). The system enables municipal operators to monitor and control a network of IoT-connected flood gates across the city of Hamburg.

## Problem Statement

Coastal and river cities face increasing flood risks. Manual operation of flood barriers is slow, error-prone, and lacks real-time situational awareness. SenseMate addresses this by providing:

- **Remote monitoring** of gate positions (open/closed/out-of-service)
- **Remote control** of gate mechanisms via downlink commands
- **Real-time alerts** for status changes and uplink events
- **Role-based access** for different operator types

## Frontend Responsibilities

The React frontend is responsible for:

| Responsibility | Implementation |
|---|---|
| User authentication & session management | JWT-based auth stored in cookies + Axios headers, backed by React Context |
| Real-time gate status visualization | Leaflet map + summary info boxes |
| Gate control (open/close/OOS) | REST API calls triggered from UI dialogs |
| Downlink command dispatch | REST API with rate limiting (10-command cap + reset) |
| Activity tracking | Chronological event log with real-time updates |
| Push notifications | STOMP over SockJS WebSocket subscriptions |
| Guest access | Unauthenticated read-only dashboard |

## User Roles

| Role | Access Level | Route |
|---|---|---|
| **Controller** | Full CRUD — can open/close gates, send downlinks, manage gates | `/dashboard` |
| **Viewer** | Read-only — can see gate status and map but cannot modify | `/dashboard-view` |
| **Guest** | No authentication — read-only with limited functionality | `/dashboard-guest` |

## Key Design Decisions

1. **No TypeScript**: The codebase is plain JavaScript (JSX). This was a deliberate choice to reduce build complexity and onboarding friction for a student team.

2. **No state management library**: Instead of Redux or Zustand, the app uses React Context for auth and local `useState`/`useEffect` for all other state. The app is flat enough that prop drilling is not a concern.

3. **WebSocket-first for real-time**: Components subscribe to STOMP topics on mount and update local state directly from messages. A polling fallback (300ms) is used in the read-only view.

4. **Feature-based folder structure**: Code is organized by domain (auth, gates, map, activities, notifications) rather than by technical layer (components, containers, etc.). This makes feature discovery intuitive.

5. **No route guards**: Authentication enforcement is done imperatively inside each page component rather than via a protected route wrapper. This allows for more flexible loading and error states.

## Backend Integration

The frontend communicates with a **Java Spring Boot** backend at `http://localhost:8080`:

- **REST API** — CRUD operations for gates, users, activities, notifications, and downlinks
- **STOMP WebSocket** — Real-time push for gate updates, activity logs, and uplink events
- **JWT Authentication** — Token-based auth with cookie persistence
