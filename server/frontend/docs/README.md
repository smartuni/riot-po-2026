# SenseMate Frontend

> **Reliable IoT (RIOT) Team — PO-2026**  
> A university project for intelligent flood disaster management

Welcome to the frontend documentation for **SenseMate**, the dashboard and control interface for an IoT-based flood gate management system. This documentation covers the architecture, technology choices, data flows, and design decisions of the React application.

## Quick Links

- [Project Overview](overview.md)
- [Tech Stack](techstack.md)
- [Architecture](architecture.md)
- [Pages & Routing](routing.md)
- [Authentication Flow](authentication.md)
- [Real-Time Communication](real-time.md)
- [Data Flow Patterns](data-flow.md)
- [Build & Deployment](deployment.md)

---

## What is SenseMate?

SenseMate is a **flood gate monitoring and control dashboard** that allows operators to:

- Monitor the real-time status of flood gates across a city (Hamburg)
- Open, close, or mark gates as out-of-service remotely
- View gate locations on an interactive Leaflet map
- Send downlink commands to IoT end-node devices
- Track gate activity in a chronological log
- Receive real-time notifications via WebSocket
- Support multiple user roles: **Controllers** (full control), **Viewers** (read-only), and **Guests** (unauthenticated read-only)

The frontend communicates with a Java Spring Boot backend over REST APIs and STOMP/WebSocket for real-time updates.

---

## Directory Layout (at a glance)

```
src/
├── app/              # Root App component & route definitions
├── pages/            # 7 top-level page components
├── features/         # Domain-driven feature modules
│   ├── auth/         # Authentication & user management
│   ├── gates/        # Flood gate CRUD & status
│   ├── map/          # Leaflet map visualization
│   ├── activities/   # Gate activity log
│   └── notifications/# User notification system
└── shared/           # Cross-cutting code
    ├── api/          # Axios HTTP client singleton
    ├── components/   # Shared UI components
    ├── styles/       # Global stylesheets
    └── utils/        # Utility functions (cookies)
```
