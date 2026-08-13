# Rescue-Mate Backend

The backend for **Rescue-Mate**, an IoT flood-gate monitoring system built as part of the RIOT project course `riot-po-2026`. It sits at the centre of the system between the field hardware and the dashboard:

```
SenseGate / SenseMate  ⇄  LoRaWAN  ⇄  TTN  ⇄  MQTT  ⇄  [ BACKEND ]  ⇄  REST + WebSocket  ⇄  React Frontend
```

- **Java 17 · Spring Boot 3.4.4 · Maven** (`com.riot:matesense`)
- Receives **uplinks** from gates and worker devices over **MQTT (The Things Network)**
- Publishes **downlinks** (gate commands / jobs) back to the devices
- Exposes a **REST API** and **STOMP WebSocket** channel to the frontend
- Persists gates, activities, notifications, nodes and keys in **PostgreSQL** managed by **Flyway**

> Architectural documentation for the whole project (arc42) lives in the [project wiki](https://github.com/smartuni/riot-po-2026/wiki).

---

## Table of Contents

- [Features](#features)
- [Architecture](#architecture)
- [Directory Structure](#directory-structure)
- [Technology Stack](#technology-stack)
- [Database](#database)
- [MQTT / TTN Integration](#mqtt--ttn-integration)
- [API Overview](#api-overview)
- [WebSockets](#websockets)
- [Security](#security)
- [Spring Profiles](#spring-profiles)
- [Running Locally](#running-locally)
- [Running with Docker](#running-with-docker)
- [Testing](#testing)
- [E2E Environment](#e2e-environment)
- [CI/CD](#cicd)
- [Troubleshooting](#troubleshooting)

---

## Features

- **Gate monitoring & control** — track gate status (`OPEN` / `CLOSED` / `OUT_OF_SERVICE`), request status changes, manual overrides, priority and height above NN.
- **Worker state confirmation** — SenseMate worker reports are aggregated into a per-gate `StateConfirmation` and a **0–100 confidence score** with a quality rating.
- **Health monitoring** — last-known health state (battery, shock/free-fall, voltage) per SenseGate, served over REST and pushed live over WebSockets.
- **Node management** — register/delete nodes and manage the singleton Ed25519 **root key** used for device identity.
- **Gate metadata** — arbitrary `key/value` metadata per gate (persistent, upsert, broadcast over WS).
- **Downlink scheduling** — build a CBOR gate-command payload and publish it to every registered gate device via TTN, rate-limited by a daily counter.
- **Authentication** — full register/login/logout with **HttpOnly JWT cookies**, server-side token store, BCrypt password hashing, CSRF protection and `controller` / `viewer` roles.
- **Deterministic E2E environment** — a self-contained profile + seed data so Playwright tests run reproducibly in CI.

---

## Architecture

The backend is organised in layers:

```
┌─────────────────────────── HTTP + WebSocket API ───────────────────────────┐
│                        AuthController · GateController ·                    │
│   controllers   HealthController · NodeManagementController · ...          │
├────────────────────────────────────────────────────────────────────────────┤
│   services      GateService · GateActivityService · DownlinkService ·      │
│                 HealthStatusService · NodeManagementService · ...          │
├────────────────────────────────────────────────────────────────────────────┤
│   mqtt          TTNMqttListener · MqttMessageHandler · TTNMqttPublisher    │
├────────────────────────────────────────────────────────────────────────────┤
│   repository / entity / model   (JPA + PostgreSQL + Flyway)                │
├────────────────────────────────────────────────────────────────────────────┤
│   security / config / exceptions / enums / registry                        │
└────────────────────────────────────────────────────────────────────────────┘
```

**Message flow (uplink, device → backend):**

1. `TTNMqttListener` receives the LoRaWAN uplink envelope from TTN via MQTT.
2. The base64 `frm_payload` is decoded and deserialised from **CBOR** (`Base64ToList`).
3. `JsonFormatter` normalises it to `{ messageType, statuses }` (IST_STATE, SEEN_TABLE_STATE or HEALTH_MONITORING).
4. `MqttMessageHandler` routes each message type: updates gate state, logs activities, recomputes confidence/state-confirmation, or stores health data.
5. Live updates are pushed to the frontend over `/topic/uplinks`, `/topic/gates/updates`, `/topic/health`, etc.

**Message flow (downlink, backend → device):**

1. `POST /downlink` (or internal services) builds a gate-command **CBOR** payload (`DownlinkService`).
2. The payload is base64-wrapped in a TTN downlink envelope (`f_port: 15`).
3. `TTNMqttPublisher` publishes it to `v3/{app}@ttn/devices/{device}/down/push` for every registered gate device.

---

## Directory Structure

```
server/backend
├── Dockerfile                     # Multi-stage build (Maven 3.9 + Temurin 17)
├── mvnw / mvnw.cmd                # Maven wrapper
├── pom.xml                        # Dependencies & build config
├── .env.example                   # Template for environment variables
├── scripts/
│   └── e2e-reset.sh               # Reset the deterministic E2E backend
└── src/
    ├── main/
    │   ├── java/com/riot/matesense/
    │   │   ├── config/            # MQTT, WebSocket, Security, Web (CORS), properties
    │   │   ├── controller/        # REST endpoints (see API Overview)
    │   │   ├── entity/            # JPA entities (Gates, Users, Nodes, ...)
    │   │   ├── enums/             # Status, ActivityType, BatteryStatus, ...
    │   │   ├── exceptions/        # Domain exceptions + ApiExceptionHandler
    │   │   ├── model/             # DTOs exchanged with the frontend
    │   │   ├── mqtt/              # TTN listener/handler/publisher
    │   │   ├── registry/          # In-memory registry of sensegate-*/sensemate-* devices
    │   │   ├── repository/        # Spring Data JPA repositories
    │   │   ├── security/          # JWT filter, cookie extractor, JwtService
    │   │   └── service/           # Business logic layer
    │   └── resources/
    │       ├── application.properties     # Default PostgreSQL + Flyway config
    │       ├── application.yml            # MQTT, test-credentials, jwt-secrets
    │       ├── application-{dev,prod,test,e2e,integration}.{properties,yml}
    │       ├── db/migration/              # Flyway migrations V1–V14
    │       ├── db/migration-e2e/          # E2E seed (PostgreSQL dialect)
    │       └── data-e2e.sql               # E2E seed (H2 dialect)
    └── test/
        ├── java/com/riot/matesense/       # Unit, repository & integration tests
        └── resources/                     # Test profile config
```

---

## Technology Stack

| Concern | Choice |
|---|---|
| Language / Runtime | Java 17 (Temurin) |
| Framework | Spring Boot 3.4.4 (Web, Data JPA, WebSocket, Security, Validation, Actuator) |
| Build | Maven 3.9 (wrapper included) |
| Database | PostgreSQL 15 + **Flyway** migrations |
| MQTT | Eclipse Paho `org.eclipse.paho.client.mqttv3` 1.2.5 |
| CBOR | Jackson `jackson-dataformat-cbor` 2.15.2 |
| Auth | jjwt 0.11.5 (HS256) + BCrypt |
| Serialisation | Jackson (JSON), Jackson CBOR |
| Tests | JUnit 5, Mockito, Spring Security Test, H2, Playwright (frontend e2e) |

---

## Database

**PostgreSQL** with **Flyway** for versioned migrations. Migrations live in `src/main/resources/db/migration/` and run automatically on startup (`spring.flyway.enabled=true`).

| Migration | Purpose |
|---|---|
| `V1__Initial_schema.sql` | Core tables (`users`, `gates`, `gate_activities`, `notifications`, `gate_for_downlink`, `downlink_counter`), indexes and views |
| `V2__Insert_seed_data.sql` | Seeds 6 Hamburg gates, notifications and activities |
| `V3__Fix_enum_array_columns.sql` | `status_enum[]` → `smallint[]` |
| `V4__Fix_quality_enum_column.sql` | `quality` enum → `VARCHAR` |
| `V5__Fix_state_confirmation_column.sql` | `state_confirmation` enum → `VARCHAR` |
| `V6__Add_updated_at_triggers.sql` | `updated_at` auto-update triggers |
| `V7__Add_foreign_key_constraints.sql` | FK constraints + cascade rules |
| `V8__Fix_status_column.sql` | `gates.status` enum → `VARCHAR` |
| `V9__Fix_notifications_status_column.sql` | `notifications.status` enum → `VARCHAR` |
| `V10__Fix_gate_activities_activity_type_column.sql` | `activity_type` enum → `VARCHAR` |
| `V11__Create_gate_metadata_table.sql` | `gate_metadata` table |
| `V12__Add_gate_override_height_fk_unique.sql` | `manual_override`, `height_above_nn`, FK cascade + unique `(gate_id, key)` |
| `V13__Add_node_management_tables.sql` | `root_keys` + `nodes` tables |
| `V14__Add_kid_and_triggers.sql` | `kid` on `root_keys`, singleton unique index, triggers |

Environment overrides via `DB_HOST`, `DB_PORT`, `DB_NAME`, `DB_USERNAME`, `DB_PASSWORD` (defaults: `postgres:5432/matesense`).

---

## MQTT / TTN Integration

The backend subscribes to uplinks and publishes downlinks through **The Things Network** over MQTT (QoS 0 for uplinks, QoS 1 for downlink pushes).

Config (`application.yml`, overridable via env vars):

```yaml
mqtt:
  broker: ssl://eu1.cloud.thethings.network:8883
  clientId: mqtt-client-1234
  username: your-app-id@ttn
  applicationId: your-app-id
  password: your-ttn-api-key
  subscribeTopic: v3/your-app-id@ttn/devices/+/up
```

- Uplink topic: `v3/{app}@ttn/devices/+/up`
- Downlink topic: `v3/{app}@ttn/devices/{deviceId}/down/push`
- Set `mqtt.enabled=false` to run without a broker (used by the `e2e` profile).
- Payloads are **CBOR**, base64-encoded by TTN and decoded on the backend.

---

## API Overview

> All endpoints are under the backend origin (port `8080`, or `/api` via the frontend nginx proxy). Paths below are the raw Spring routes.

### Auth — `/auth`

| Method | Path | Description | Auth |
|---|---|---|---|
| POST | `/auth/login` | Login, sets HttpOnly JWT cookie | public |
| POST | `/auth/register` | Create account, sets JWT cookie | public |
| POST | `/auth/logout` | Invalidate token & clear cookie | public |
| GET | `/auth/user-details` | Current user details | authenticated |
| PUT | `/auth/user-change` | Update name / password | authenticated |

### Gates

| Method | Path | Description | Auth |
|---|---|---|---|
| GET | `/gates` | All gates | public |
| POST | `/add-gate-ui` | Create a gate from the UI | authenticated |
| PUT | `/update-gate` | Update a gate | authenticated |
| DELETE | `/gates/{id}` | Delete a gate | authenticated |
| POST | `/{gateId}/{workerId}/request-status-change/` | Request target status | authenticated |
| GET | `/gates_for_downlink` | Gates mapped for downlink commands | authenticated |
| PUT | `/update-priority/{gateId}` | Update gate priority | authenticated |
| PUT | `/update-height/{gateId}` | Update height above NN | authenticated |
| POST | `/gates/{gateId}/{workerId}/set-status` | Manual status override | authenticated |

### Gate Metadata

| Method | Path | Description | Auth |
|---|---|---|---|
| GET | `/gates/{gateId}/metadata` | List metadata for a gate | authenticated |
| POST | `/gates/{gateId}/metadata` | Add/upsert metadata entry | authenticated |
| PUT | `/gates/{gateId}/metadata/{metadataId}` | Update entry | authenticated |
| DELETE | `/gates/{gateId}/metadata/{metadataId}` | Delete entry | authenticated |

### Health

| Method | Path | Description | Auth |
|---|---|---|---|
| GET | `/health` | Last-known health per SenseGate | public |

### Node Management

| Method | Path | Description | Auth |
|---|---|---|---|
| GET | `/nodes` | List registered nodes | authenticated |
| POST | `/nodes` | Register a node (Ed25519 public key) | `controller` |
| DELETE | `/nodes/{id}` | Delete a node | `controller` |
| POST | `/nodes/root-key` | Upload/upsert root key pair | `controller` |
| GET | `/nodes/root-key` | Get root key (private key redacted) | `controller` |

### Downlink

| Method | Path | Description | Auth |
|---|---|---|---|
| POST | `/downlink` | Prepare & publish a gate-command downlink | authenticated |
| GET | `/downlinkcounter/counter` | Current downlink counter | authenticated |
| POST | `/downlinkcounter/try-increment` | Increment if below limit (10/day) | authenticated |
| POST | `/downlinkcounter/reset` | Reset counter | `controller` |

### Activities & Notifications

| Method | Path | Description | Auth |
|---|---|---|---|
| GET | `/gate-activities` | All activities | public |
| POST | `/add-activity` / `/add-activities` | Log activity(ies) | authenticated |
| DELETE | `/delete-activitiy` | Delete an activity | authenticated |
| GET | `/activities/{gateId}` | Activities for a gate | authenticated |
| GET | `/activities-latest/{gateId}` | Latest worker report per gate | authenticated |
| GET | `/notifications` | All notifications | authenticated |
| POST | `/notifications/add` | Create notification | authenticated |
| DELETE | `/notifications/delete` | Delete notification | authenticated |
| GET | `/notifications/{workerId}` | Notifications for a worker | authenticated |
| POST | `/notifications/{notificationId}/request-read-change` | Mark read/unread | authenticated |

### Misc

| Method | Path | Description | Auth |
|---|---|---|---|
| GET | `/` | Health banner | public |
| GET | `/secured` | Auth check | authenticated |
| GET | `/actuator/health` | Actuator health (used by Docker HEALTHCHECK) | public |

> The `/e2e/**` endpoints (`simulate-uplink`, `simulate-state-confirmation`, `simulate-health`) exist **only** under the `e2e` profile for deterministic testing.

---

## WebSockets

STOMP endpoint at `/ws` (native WebSocket, no SockJS), simple broker on `/topic`, app prefix `/app`. JWT is validated on the STOMP `CONNECT` frame.

| Topic | Payload |
|---|---|
| `/topic/gates/updates` | Gate state changes |
| `/topic/gates/delete` | Deleted gates |
| `/topic/uplinks` | Processed uplink messages |
| `/topic/health` | Live health updates |
| `/topic/gate-metadata/{gateId}` | Metadata create/update |
| `/topic/gate-metadata/{gateId}/delete` | Metadata deletion |
| `/topic/notifications` | Notification updates |

---

## Security

- **JWT auth** — HS256 tokens stored in an **HttpOnly, SameSite=Lax cookie** (`jwt`); server-side token store with scheduled eviction; logout truly invalidates.
- **CSRF** — enabled via `CookieCsrfTokenRepository` (login/logout/e2e exempt).
- **WebSocket auth** — the same JWT cookie is validated by a STOMP `CONNECT` interceptor; the handshake forwards the `Cookie` header into session attributes for browsers.
- **Roles** — `controller` (admin: node/root-key management, counter reset, metadata mutations) and `viewer`.
- **Passwords** — BCrypt hashing.
- **CORS** — globally open (`allowedOriginPatterns("*")`) for development.
- **Production hardening** — `prod` profile forces `Secure` cookies and suppresses error details; set `JWT_SECRET` to a strong 64+ char value.

---

## Spring Profiles

| Profile | Database | Flyway | Notes |
|---|---|---|---|
| *(default)* | PostgreSQL | on | `ddl-auto=none`, Flyway `classpath:db/migration` |
| `dev` | PostgreSQL | on | Verbose logging, SQL output |
| `prod` | PostgreSQL | on | Secure cookies, no error details |
| `test` | H2 in-memory | off | `create-drop`, used by unit/repository tests |
| `e2e` | H2 in-memory | off | Deterministic seed (`data-e2e.sql`), MQTT disabled, `/e2e/**` active |
| `integration` | PostgreSQL | on | `ddl-auto=validate`, env-gated (`RUN_POSTGRES_INTEGRATION_TESTS=true`) |

---

## Running Locally

**Prerequisites:** Java 17, Maven (or the included `./mvnw`), PostgreSQL 15 (or Docker).

1. Create the database and copy `.env.example` to `.env`; export the values (or rely on defaults `postgres/postgres`).
2. Configure MQTT in `application.yml` (see [MQTT / TTN Integration](#mqtt--ttn-integration)).
3. Start PostgreSQL: `docker compose -f server/docker-compose.yml up -d postgres`
4. Run the app:

```bash
cd server/backend
./mvnw spring-boot:run -Dspring-boot.run.profiles=dev
```

The API is now at `http://localhost:8080`.

---

## Running with Docker

```bash
# Full stack (postgres + backend + frontend)
docker compose -f server/docker-compose.yml up -d --build

# Backend only
docker compose -f server/docker-compose.yml up -d --build postgres backend

# Dev profile override
docker compose -f server/docker-compose.yml -f server/docker-compose.dev.yml up -d --build
```

| Service | Port | Notes |
|---|---|---|
| `postgres` | `5432` | PostgreSQL 15, volume `postgres_data` |
| `backend` | `8080` | `SPRING_PROFILES_ACTIVE=prod`, HEALTHCHECK via `/actuator/health` |
| `frontend` | `3000` | nginx, proxies to `backend:8080` |

---

## Testing

```bash
cd server/backend
./mvnw test                                  # Unit + repository tests (H2)
./mvnw verify -DskipTests=false              # Full build
# PostgreSQL integration tests (requires a running Postgres):
RUN_POSTGRES_INTEGRATION_TESTS=true ./mvnw test
```

Coverage includes:
- `HealthControllerTest`, `HealthStatusIntegrationTest` — health REST + CBOR parsing
- `NodeManagementServiceTest`, `NodeRepositoryTest`, `RootKeyRepositoryTest` — node & key management
- `UserRepositoryTest` — auth persistence
- `PostgreSqlMigrationIntegrationTest` — Flyway migrations against a real PostgreSQL

Frontend e2e tests (Playwright) run against the `e2e` backend in CI.

---

## E2E Environment

For reproducible end-to-end tests there is a dedicated `e2e` profile with deterministic seed data (users `test@example.com/test123` with `controller` role, `test2@example.com/test234` as viewer; gates `1001`–`1004`).

```bash
# Reset to a clean, freshly-seeded state
server/backend/scripts/e2e-reset.sh
# or manually:
docker compose -f server/docker-compose.e2e.yml down
docker compose -f server/docker-compose.e2e.yml up -d --build backend
```

- Backend: `localhost:8080` · Postgres (e2e): `localhost:5433`
- MQTT is disabled (`mqtt.enabled=false`) so tests run deterministically.
- Simulation endpoints under `/e2e/**` let Playwright inject uplinks, state confirmations and health updates.

---

## CI/CD

GitHub Actions pipeline builds and tests the backend, spins up PostgreSQL, runs Flyway migrations and integration tests, then runs the frontend Playwright suite against the deterministic `e2e` backend. See `.github/workflows/` for details.

---

## Troubleshooting

**MQTT connection fails despite correct setup**
Network restrictions (corporate firewall/university Wi-Fi) or a blocked port `8883` can prevent the TLS connection. Try a different network, verify the API key, and ensure the system clock is accurate.

**`application.yml` settings ignored**
Make sure `src/main/resources` is marked as a **Resource Root** (IntelliJ: right-click → *Mark Directory as* → *Resources Root*).

**Maven dependencies not resolving**
Import the project as a Maven project (IntelliJ: right-click `pom.xml` → *Add as Maven Project*) and run `./mvnw clean install`.

**Database migrations fail on startup**
PostgreSQL must be reachable on `5432` before the backend starts (the Docker Compose `depends_on: condition: service_healthy` handles this in containers). Use `docker compose logs postgres` to check readiness.

**401 on WebSocket connection**
The STOMP `CONNECT` frame must carry the JWT cookie. If testing with a tool, include the `jwt` cookie value in the connection headers.

---

> **Security note:** `src/main/resources/application.yml` currently contains a committed TTN API key and default credentials. These should be moved to environment variables / secrets before production deployment (see `.env.example`).
