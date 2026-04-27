# Test Data & Seed Data Configuration

## Overview

This document explains how test data is managed in the Riot-PO application.

## Data Initialization Strategy

### Production Environment
- **No test data** is inserted
- Only `V1__Initial_schema.sql` migration runs
- Fresh database starts empty

### Development Environment (`dev` profile)
- **Seed data is inserted** via Flyway script `V2__Insert_seed_data.sql`
- Additionally, `PopulateTestDataRunner` runs if profile is active
- Test users, gates, and notifications are pre-populated

### Testing Environment
- **H2 in-memory database** with fresh schema (via `V1__Initial_schema.sql`)
- No seed data from V2 (can be customized per test)
- Each test gets isolated database state

---

## How It Works

### 1. Flyway Migrations

#### V1__Initial_schema.sql
```
✅ Always runs (all environments)
✅ Creates tables, ENUMs, views, indexes
✅ Inserts default downlink counter (required for app)
❌ Does NOT insert test user data
```

#### V2__Insert_seed_data.sql
```
✅ Runs in ALL environments (currently)
✅ Inserts sample users, gates, notifications
✅ Useful for manual testing
❌ Should NOT be in production ideally
```

**Note:** Currently V2 always runs. To make it conditional for dev-only:
- Flyway doesn't have built-in profile support
- Solution: Use separate migration script or environment variable

### 2. CommandLineRunner Component

**File:** `Application.java`
**Class:** `PopulateTestDataRunner`

```java
@Component
@Profile("dev")  // ← Only runs with --spring.profiles.active=dev
class PopulateTestDataRunner implements CommandLineRunner
```

**What it does:**
- Creates additional test notifications
- Runs test seed logic at startup
- Only active in development profile

**When it runs:**
- Local development with `mvn spring-boot:run`
- Docker container with `SPRING_PROFILES_ACTIVE=dev`

---

## Usage

### Local Development with Test Data

Start with dev profile to get test data:

```bash
# Using Maven
mvn spring-boot:run -Dspring-boot.run.arguments="--spring.profiles.active=dev"

# Or set via environment
export SPRING_PROFILES_ACTIVE=dev
mvn spring-boot:run

# Or via Java system property
java -jar app.jar -Dspring.profiles.active=dev
```

### Docker Local Development with Test Data

In `docker-compose.yml`:
```yaml
services:
  backend:
    environment:
      SPRING_PROFILES_ACTIVE: dev
```

Then start:
```bash
docker-compose up --build
```

### Production (No Test Data)

```bash
# Default profile (no -Dspring.profiles.active)
java -jar app.jar

# Or explicitly set to empty
java -jar app.jar -Dspring.profiles.active=
```

### Running Tests

Tests use `application-test.properties`:
```
spring.profiles.active=test
spring.datasource.url=jdbc:h2:mem:testdb
spring.flyway.enabled=false
```

Run tests:
```bash
mvn clean test
mvn test -Dspring-boot.run.arguments="--spring.profiles.active=test"
```

---

## Test Data Available

### Users (from V2__Insert_seed_data.sql)
- **test@example.com** / role: controller (for testing all features)
- **viewer@example.com** / role: viewer (for testing limited access)

Password: `test123` (hashed in DB)

### Gates
- 6 sample gates with different statuses:
  - Gates 1, 2, 4, 6: OPEN
  - Gates 3, 5: CLOSED
  - All have coordinates in Hamburg area

### Notifications
- 5 sample notifications with different states

### Gate Activities (Audit Log)
- 4 sample activities showing different event types

---

## Modifying Test Data

### Add More Users

Edit `V2__Insert_seed_data.sql`:
```sql
INSERT INTO users (email, password, name, role) VALUES
('newuser@example.com', '<hashed_password>', 'New User', 'controller')
ON CONFLICT (email) DO NOTHING;
```

### Add More Gates

Edit `V2__Insert_seed_data.sql`:
```sql
INSERT INTO gates (id, status, location, latitude, longitude, ...)
VALUES (7, 'OPEN'::status_enum, 'New Location', 53.5500, 9.9000, ...)
ON CONFLICT (id) DO NOTHING;
```

### Custom Test Data for Tests

Create test-specific fixtures in your test class:
```java
@Before
public void setup() {
    UserEntity testUser = new UserEntity("test@local.test", "password", "Test", "controller");
    userRepository.save(testUser);
}
```

---

## Best Practices

1. **Never modify committed Flyway scripts**
   - Create new migration files instead
   - Use `V3__Add_more_seed_data.sql` for additions

2. **Keep test data realistic**
   - Use real-world coordinates and names
   - Don't use placeholder values like "test123"

3. **Version control seed data**
   - Commit migration scripts to git
   - Reproducible across environments

4. **Profile-based configuration**
   - Dev profile: verbose logging + test data
   - Test profile: minimal setup
   - Production: clean start

5. **Database cleanup**
   - Tests use H2 (fresh each time)
   - Local dev persists data (use `docker-compose down` to reset)

---

## Troubleshooting

### Problem: Test data not inserted
```
✓ Check if dev profile is active: SPRING_PROFILES_ACTIVE=dev
✓ Check Flyway logs for V2__Insert_seed_data.sql execution
✓ Verify PostgreSQL is running and accessible
```

### Problem: Test users not working
```
✓ Verify password hash in V2 script is correct
✓ Check user roles are: 'controller' or 'viewer'
✓ Ensure users table exists
```

### Problem: Application fails on startup
```
✓ H2 for tests might be missing: check pom.xml scope=test
✓ PostgreSQL connection issues in dev: check .env file
✓ Flyway migration syntax error: check SQL script
```

---

## Related Files

- `Application.java` - PopulateTestDataRunner component
- `application.properties` - Default (production) config
- `application-test.properties` - Test profile config
- `application-dev.properties` - Dev profile config
- `db/migration/V1__Initial_schema.sql` - Schema creation
- `db/migration/V2__Insert_seed_data.sql` - Seed data

## Support

For questions about test data configuration, contact the Backend Team.

