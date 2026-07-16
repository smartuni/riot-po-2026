# 04 — API Endpoints

## Overview

The Backend exposes a **REST API** that the React frontend calls to perform all operations. Most endpoints return and accept **JSON** data. Authentication is handled via a JWT stored in an HTTP-only cookie — the browser sends it automatically with every request.

---

## Auth Endpoints

**Controller:** `server/backend/src/main/java/com/riot/matesense/controller/AuthController.java:17-97`

These endpoints handle user authentication — login, registration, logout, and profile management. **All auth endpoints are public** (no authentication required to access them).

| Method | Path | Purpose |
|---|---|---|
| `POST` | `/auth/login` | Log in with email + password. Returns user details and sets a JWT cookie. |
| `POST` | `/auth/register` | Register a new user account. Returns user details and sets a JWT cookie. |
| `POST` | `/auth/logout` | Log out. Clears the JWT cookie and invalidates the token server-side. |
| `GET` | `/auth/user-details` | Get the current logged-in user's details (name, email, role, ID). |
| `PUT` | `/auth/user-change` | Change the current user's name and/or password. Requires current password for verification. |

### Login Flow
1. Frontend sends `POST /auth/login` with `{ "email": "...", "password": "..." }`
2. `AuthController.login()` (`server/backend/src/main/java/com/riot/matesense/controller/AuthController.java:28-39`) calls `AuthService.handleLogin()`
3. If credentials match, a JWT is generated and set as an HTTP-only cookie named `jwt`
4. User details are returned as JSON

### Roles
Users have one of two roles stored in the `role` column:
- **`controller`** — Full access: manage gates, nodes, root keys, reset downlink counters
- **`viewer`** — Read-only access: view gates, activities, health data, notifications

Role-based access is enforced via `@PreAuthorize` annotations on controller methods (e.g., `NodeManagementController.java:24`).

---

## Gate Endpoints

**Controller:** `server/backend/src/main/java/com/riot/matesense/controller/GateController.java:25-163`

These endpoints manage floodgate data — listing, adding, updating, and deleting gates, as well as requesting status changes.

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/gates` | Get all gates with current status, location, confidence, etc. |
| `POST` | `/add-gate-ui` | Add a new gate from the web UI |
| `PUT` | `/update-gate` | Update an existing gate's properties |
| `DELETE` | `/gates/{id}` | Delete a gate by its ID |
| `POST` | `/{gateId}/{workerId}/request-status-change/` | Request a gate status change (e.g., "request to open") |
| `POST` | `/gates/{gateId}/{workerId}/set-status` | Manually override a gate's status (direct, not a request) |
| `GET` | `/gates_for_downlink` | Get all gates formatted for downlink (compact, just ID + requested status) |
| `PUT` | `/update-priority/{gateId}` | Change a gate's display priority |
| `PUT` | `/update-height/{gateId}` | Update a gate's elevation (height above sea level) |

### Key Concepts

- **Requested Status:** A user can *request* a gate to open or close. This does not immediately change the gate; it sets a `requestedStatus` that gets sent to the device via downlink. The actual status only changes when the sensor confirms it.
- **Manual Override:** The `/set-status` endpoint directly changes the gate's status and sets `manualOverride = true`. This is for emergency situations where the sensor data should be overridden.
- **Priority:** Gates can be assigned a priority for ordering in the dashboard UI. Higher numbers = higher priority.

---

## Gate Activity Endpoints

**Controller:** `server/backend/src/main/java/com/riot/matesense/controller/GateActivityController.java:12-48`

Gate activities are an **audit log** — every change to a gate's state is recorded as an activity entry. This provides a historical timeline of what happened to each gate.

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/gate-activities` | Get all activity entries across all gates |
| `POST` | `/add-activity` | Add a single activity entry |
| `POST` | `/add-activities` | Add multiple activity entries at once |
| `DELETE` | `/delete-activitiy` | Delete an activity entry |
| `GET` | `/activities/{gateId}` | Get all activities for a specific gate |
| `GET` | `/activities-latest/{gateId}` | Get the most recent activities for a specific gate |

### Activity Types
Each activity has an `ActivityType` (`server/backend/src/main/java/com/riot/matesense/enums/ActivityType.java:3-9`):
- `SENSOR_NEW` — A new gate was discovered from sensor data
- `SENSOR_VALUE_CHANGED` — A gate's sensor reported a different status
- `SENSOR_VALUE_KEEPALIVE` — A gate confirmed its current status (no change)
- `SENSEMATE_WORKER_REPORT` — A field worker reported a gate's status via SenseMate
- `TARGET_STATE_REQUEST` — A user requested a gate to open/close via the dashboard
- `MANUAL_STATUS_SET` — A user manually overrode a gate's status

---

## Gate Metadata Endpoints

**Controller:** `server/backend/src/main/java/com/riot/matesense/controller/GateMetadataController.java:15-76`

Metadata provides a flexible way to attach arbitrary **key-value pairs** to gates (e.g., `"manufacturer" → "Siemens"`, `"maintenance_date" → "2026-03-01"`). This avoids having to add new database columns for every new attribute.

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/gates/{gateId}/metadata` | Get all metadata entries for a gate |
| `POST` | `/gates/{gateId}/metadata` | Add a metadata entry (key + value) |
| `PUT` | `/gates/{gateId}/metadata/{metadataId}` | Update a metadata entry |
| `DELETE` | `/gates/{gateId}/metadata/{metadataId}` | Delete a metadata entry |

---

## Node Management Endpoints

**Controller:** `server/backend/src/main/java/com/riot/matesense/controller/NodeManagementController.java:13-51`

Nodes represent the physical IoT devices (SenseGate and SenseMate) registered in the system. Each node has a name and a public key for cryptographic verification.

| Method | Path | Purpose | Access |
|---|---|---|---|
| `GET` | `/nodes` | List all registered nodes | Any authenticated user |
| `POST` | `/nodes` | Register a new node (name + public key) | `controller` role only |
| `DELETE` | `/nodes/{id}` | Delete a node | `controller` role only |
| `POST` | `/nodes/root-key` | Upload or update the root key (KID + public/private keys) | `controller` role only |
| `GET` | `/nodes/root-key` | Get the current root key | `controller` role only |

The **root key** is a cryptographic key pair used to sign COSE messages between the server and devices. It consists of:
- `kid` — Key Identifier
- `publicKey` — Public key string
- `privateKey` — Private key string

---

## Notification Endpoints

**Controller:** `server/backend/src/main/java/com/riot/matesense/controller/NotificationController.java:13-80`

Notifications inform workers about gate-related events (e.g., "Worker 1 shall close Gate 5"). Each notification is associated with a `workerId` and can be marked as read.

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/notifications` | Get all notifications |
| `POST` | `/notifications/add` | Create a new notification |
| `DELETE` | `/notifications/delete` | Delete a notification |
| `GET` | `/notifications/{workerId}` | Get notifications for a specific worker |
| `POST` | `/notifications/{notificationId}/request-read-change` | Mark a notification as read |

---

## Downlink Endpoints

**Controller:** `server/backend/src/main/java/com/riot/matesense/controller/DownlinkController.java:11-27`

Downlinks are commands sent from the server **to** a device (e.g., "gate 3 should open").

| Method | Path | Purpose |
|---|---|---|
| `POST` | `/downlink` | Send a downlink command to one or more gate devices |

**Controller:** `server/backend/src/main/java/com/riot/matesense/controller/DownlinkCounterController.java:12-48`

The downlink counter tracks how many downlinks have been sent, enforcing a rate limit.

| Method | Path | Purpose | Access |
|---|---|---|---|
| `GET` | `/downlinkcounter/counter` | Get the current counter value | Any authenticated |
| `POST` | `/downlinkcounter/try-increment` | Try to increment (returns false if limit reached) | Any authenticated |
| `POST` | `/downlinkcounter/reset` | Reset the counter to zero | `controller` role only |

---

## Health Endpoints

**Controller:** `server/backend/src/main/java/com/riot/matesense/controller/HealthController.java:10-24`

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/health` | Get health status for all known SenseGate devices (battery, shock, voltage, version) |

Health data is stored **in-memory** in `HealthStatusService` (`server/backend/src/main/java/com/riot/matesense/service/HealthStatusService.java:18`) — it's not persisted to the database. This is because health data is ephemeral (push-only from MQTT uplinks) and only the latest value matters.

### Actuator Endpoints
Spring Boot Actuator provides built-in system monitoring endpoints:

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/actuator/health` | Basic health check — returns `{"status":"UP"}` if the backend is running |

---

## DTOs vs Entities

A key distinction in the codebase:

| | **Entity** | **DTO (Model)** |
|---|---|---|
| **Location** | `entity/` package | `model/` package |
| **Purpose** | Maps directly to a database table row | Transfers data between layers (e.g., API response) |
| **Database** | Yes — stored in PostgreSQL | No |
| **Example** | `GateEntity` has 23 fields matching `gates` table columns | `Gate` model has only the fields the frontend needs |
| **Format** | May contain JPA annotations, internal logic | Plain data holder, often with a constructor |

### Why the distinction?

- **Security:** Entities may contain sensitive fields (e.g., password hashes). DTOs can exclude these.
- **Flexibility:** You can combine data from multiple entities into one DTO for a specific API response.
- **Decoupling:** Changing the database schema doesn't necessarily break the API contract.

Entities often have `toModel()` or similar methods to convert themselves into DTOs. For example, in `GateService.getAllGates()` (`server/backend/src/main/java/com/riot/matesense/service/GateService.java:41-52`), each `GateEntity` is converted to a `Gate` model before being returned to the controller.

---

Next: **[05-database.md](05-database.md)** — Database structure and Flyway migrations.
