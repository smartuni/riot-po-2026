# 02 — Setup

## Prerequisites

Before running the Backend, you need:

| Requirement | Version/Details |
|---|---|
| **Java Development Kit (JDK)** | 17 or higher |
| **Maven** | Included via `./mvnw` wrapper — no global install needed |
| **PostgreSQL** | 15 (for `dev` and `prod` profiles) |
| **Docker** (optional) | For running the full stack with Docker Compose |
| **TTN Account** | Required for MQTT connectivity to The Things Network |

---

## Step 1: Clone the Repository

```bash
git clone --recurse-submodules https://github.com/.../BAI5-Project_RIOT26.git
cd BAI5-Project_RIOT26
```

If you already cloned without submodules, run:
```bash
./submodules_linux.sh
```

---

## Step 2: Set Up Environment Variables

Copy the example environment file and customize it:

```bash
cp server/backend/.env.example server/backend/.env
```

The `.env.example` (`server/backend/.env.example:1-37`) contains these settings:

| Variable | Purpose | Default |
|---|---|---|
| `POSTGRES_DB` | Database name | `riot_db` |
| `POSTGRES_USER` | Database user | `riot_user` |
| `POSTGRES_PASSWORD` | Database password | `riot_password` |
| `DB_URL` | JDBC connection string | `jdbc:postgresql://postgres:5432/riot_db` |
| `SPRING_PROFILES_ACTIVE` | Active Spring profile | `dev` |
| `MQTT_USERNAME` | TTN application username | `testing-area-1@ttn` |
| `JWT_SECRET` | Secret key for signing JWT tokens | *(change this!)* |

**Important:** Change the `JWT_SECRET` to a strong random value (64+ characters). You can generate one with:
```bash
python3 -c "import secrets; print(secrets.token_urlsafe(64))"
```

---

## Step 3: Configure MQTT (TTN)

The Backend connects to The Things Network via MQTT to receive sensor data. You need a TTN application and API key.

Edit `server/backend/src/main/resources/application.yml` (`server/backend/src/main/resources/application.yml:1-7`) with your TTN credentials:

```yaml
mqtt:
  broker: ssl://eu1.cloud.thethings.network:8883
  clientId: mqtt-client-1234
  username: your-app-id@ttn
  applicationId: your-app-id
  password: your-ttn-api-key
  subscribeTopic: v3/your-app-id@ttn/devices/+/up
```

The `subscribeTopic` uses the `+` wildcard to listen for uplinks from **all** devices registered in your TTN application.

---

## Step 4: Set Up Test Credentials

The `test-credentials` section in `application.yml` (`server/backend/src/main/resources/application.yml:9-20`) defines accounts that are automatically created when the `dev` profile is active:

```yaml
test-credentials:
  accounts:
    - username: test
      password: test123
      email: test@example.com
      role: controller
    - username: test2
      password: test234
      email: test2@example.com
      role: viewer
```

- **controller** role: Full access (manage gates, nodes, downlinks, root keys)
- **viewer** role: Read-only access (view gates, activities, health)

In production, remove or disable these test accounts.

---

## Spring Profiles

The Backend uses **Spring profiles** to adapt its behavior for different environments. The active profile is set via the `SPRING_PROFILES_ACTIVE` environment variable.

| Profile | Database | MQTT | Seed Data | Use Case |
|---|---|---|---|---|
| `dev` | PostgreSQL | enabled | Test users from `application.yml` | Local development |
| `prod` | PostgreSQL | enabled | — | Production deployment |
| `e2e` | H2 (in-memory) | **disabled** | `V100__e2e_seed.sql` | End-to-end testing |
| `integration` | PostgreSQL | enabled | — | Migration integration tests |

The `e2e` profile disables MQTT so tests run in a stable, isolated environment. The `integration` profile is configured in `server/backend/src/main/resources/application-integration.yml` (`server/backend/src/main/resources/application-integration.yml:1-15`).

---

## Running the Backend

### Option A: Direct with Maven (recommended for development)

```bash
# Development (needs PostgreSQL running)
./mvnw spring-boot:run -Dspring-boot.run.profiles=dev
```

### Option B: Docker Compose (full stack)

```bash
# Production setup (PostgreSQL + Backend + Frontend)
docker compose -f server/docker-compose.yml up -d

# Development setup (with dev profile)
docker compose -f server/docker-compose.yml -f server/docker-compose.dev.yml up -d

# E2E testing setup (backend only, H2 database, no MQTT)
docker compose -f server/docker-compose.yml -f server/docker-compose.e2e.yml up -d postgres backend
```

The Docker Compose files are:
- `server/docker-compose.yml` (`server/docker-compose.yml:1-49`) — Base: PostgreSQL 15, Backend, Frontend
- `server/docker-compose.dev.yml` (`server/docker-compose.dev.yml:1-17`) — Override: sets `SPRING_PROFILES_ACTIVE=dev`
- `server/docker-compose.e2e.yml` (`server/docker-compose.e2e.yml:1-9`) — Override: sets `SPRING_PROFILES_ACTIVE=e2e`

### Ports

| Service | Port | Notes |
|---|---|---|
| Backend | `8080` | REST API and WebSocket |
| PostgreSQL | `5432` (Docker) / `5433` (E2E) | Database |
| Frontend | `3000` (Docker) | Nginx serving static files |

---

## Running Tests

### Unit Tests

```bash
cd server/backend
./mvnw test
```

Unit tests use Spring Boot's test framework and do **not** require a database connection.

### Integration Tests

```bash
export RUN_POSTGRES_INTEGRATION_TESTS=true
./mvnw -Dtest=PostgreSqlMigrationIntegrationTest test
```

Requires PostgreSQL running. The single integration test (`PostgreSqlMigrationIntegrationTest`) verifies that all Flyway migrations apply cleanly.

### E2E Tests (Backend)

```bash
# Reset to clean state
server/backend/scripts/e2e-reset.sh

# Or manually:
docker compose -f server/docker-compose.yml -f server/docker-compose.e2e.yml down -v
docker compose -f server/docker-compose.yml -f server/docker-compose.e2e.yml up -d postgres backend
```

The E2E backend runs on port `8080` with these test accounts:
- `test@example.com` / `test123` (role: controller)
- `test2@example.com` / `test234` (role: viewer)

---

## Common Troubleshooting

### Maven project not recognized
In IntelliJ IDEA: Right-click `pom.xml` → **Add as Maven Project**. Run `./mvnw clean install` to verify the build.

### `application.yml` not loaded
Ensure `src/main/resources/` is marked as a **Resources Root** in your IDE. In IntelliJ: right-click the `resources` folder → **Mark Directory as** → **Resources Root**.

### MQTT connection fails despite correct credentials
- Try a different network (corporate/university firewalls may block port 8883)
- Ensure your system clock is accurate (TLS requires time sync)
- Check that port **8883** is not blocked

### PostgreSQL connection refused
Verify PostgreSQL is running:
```bash
pg_isready -h localhost -p 5432
```
If using Docker, check with: `docker ps | grep matesense-db`

### Test accounts not created automatically
Test accounts are only created when the `dev` profile is active (`server/backend/src/main/java/com/riot/matesense/Application.java:32-70`). They are populated by the `PopulateTestDataRunner` class which runs on application startup.

---

Next: **[03-architecture.md](03-architecture.md)** — How the code is organized and how layers work together.
