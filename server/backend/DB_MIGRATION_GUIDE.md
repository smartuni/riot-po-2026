# Database Migration Guide

## Overview

This backend uses **Flyway** for database version control and schema migrations. All database changes are tracked and applied automatically when the application starts.

## Database Configuration

### PostgreSQL Setup (Production/Development)

The application is configured to use PostgreSQL. Configuration is in `application.properties`:

```properties
spring.datasource.url=jdbc:postgresql://localhost:5432/riot_db
spring.datasource.username=riot_user
spring.datasource.password=riot_password
spring.jpa.hibernate.ddl-auto=validate
spring.flyway.enabled=true
```

### H2 Setup (Testing)

For unit and integration tests, the application uses an in-memory H2 database (see `application-test.properties`).

## Migration Files Location

All migration files are stored in:
```
server/backend/src/main/resources/db/migration/
```

## Flyway Naming Convention

Migration files follow this naming pattern:
```
V{VERSION}__{DESCRIPTION}.sql
```

Where:
- `V` = Required prefix
- `{VERSION}` = Version number (e.g., 1, 2, 3, etc.)
- `__` = Double underscore separator
- `{DESCRIPTION}` = Descriptive name (underscores replace spaces)

### Examples:
- `V1__Initial_schema.sql` - Creates all initial tables
- `V2__Add_user_timestamps.sql` - Adds created_at and updated_at columns
- `V3__Create_gate_index.sql` - Creates performance indexes

## Current Migrations

### V1__Initial_schema.sql
**Status:** Active  
**Date:** 2026-04-27  
**Changes:**
- Create ENUM types: `status_enum`, `state_confirmation_enum`, `confidence_quality_enum`, `activity_type_enum`
- Create tables: `users`, `gates`, `gate_activities`, `notifications`, `gate_for_downlink`, `downlink_counter`
- Add indexes for performance optimization
- Create helpful views: `v_gate_summary`, `v_recent_activities`
- Insert seed test data

**Tables Created:**
- `users` - User authentication and role management
- `gates` - Gate/sensor state and configuration
- `gate_activities` - Audit log of all gate changes
- `notifications` - Event notifications for users
- `gate_for_downlink` - Queue for downlink operations
- `downlink_counter` - Downlink operation counter

## Running Migrations

### Automatic (Recommended)
Flyway automatically runs migrations when the application starts:
```bash
mvn spring-boot:run
```

### Manual via Maven
```bash
mvn flyway:migrate -Dflyway.url=jdbc:postgresql://localhost:5432/riot_db \
                   -Dflyway.user=riot_user \
                   -Dflyway.password=riot_password
```

### Check Migration Status
```bash
mvn flyway:info -Dflyway.url=jdbc:postgresql://localhost:5432/riot_db \
                -Dflyway.user=riot_user \
                -Dflyway.password=riot_password
```

## Docker Development Setup

1. Start PostgreSQL with docker-compose:
```bash
docker-compose up -d postgres
```

2. Verify the database is running:
```bash
docker exec -it riot-postgres psql -U riot_user -d riot_db
```

3. Run the Spring Boot application:
```bash
mvn spring-boot:run
```

The migrations will automatically execute during startup.

## Adding New Migrations

### Step 1: Create New Migration File
Create a new file in `src/main/resources/db/migration/`:
```
V{NEXT_VERSION}__{DESCRIPTION}.sql
```

Example:
```sql
-- V2__Add_api_keys_table.sql
CREATE TABLE api_keys (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    key_hash VARCHAR(255) NOT NULL UNIQUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_used TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX idx_api_keys_user_id ON api_keys(user_id);
```

### Step 2: Test the Migration
```bash
# Run tests with fresh database
mvn clean test -Dspring.profiles.active=test

# Or start application and verify
mvn spring-boot:run
```

### Step 3: Commit and Document
```bash
git add src/main/resources/db/migration/V2__Add_api_keys_table.sql
git commit -m "feat: #5 add api_keys table for token management"
```

## Troubleshooting

### Migration Failed - "Flyway Validation Error"
**Problem:** You've modified a migration file that was already applied.

**Solution:** 
1. Never modify committed migration files
2. Create a new migration file instead
3. If in development only, use `flyway:clean` (development only!):
```bash
mvn flyway:clean -Dflyway.url=jdbc:postgresql://localhost:5432/riot_db
```

### Connection Refused
**Problem:** Cannot connect to PostgreSQL database.

**Solution:**
1. Verify PostgreSQL is running: `docker ps | grep postgres`
2. Check credentials in `application.properties`
3. Check if database and user exist:
```bash
docker exec -it riot-postgres psql -U riot_user -l
```

### Migration Not Applied
**Problem:** Migration file exists but didn't run.

**Solution:**
1. Verify filename follows convention: `V{NUMBER}__{NAME}.sql`
2. Check file is in correct location: `src/main/resources/db/migration/`
3. Check application logs for Flyway messages:
```bash
mvn spring-boot:run -Dlogging.level.org.flywaydb=DEBUG
```

## Best Practices

1. **Always use version control** - Never modify committed migration files
2. **Write idempotent migrations** - Migrations should be safe to run multiple times
3. **Test before committing** - Run tests with fresh database
4. **Keep migrations focused** - One logical change per migration file
5. **Document complex changes** - Add comments explaining why
6. **Use descriptive names** - File names should reflect the change
7. **Include rollback strategy** - Document how to handle rollback if needed

## Related Documentation

- [Flyway Official Documentation](https://flywaydb.org/documentation/concepts/migrations)
- [Spring Boot Flyway Integration](https://docs.spring.io/spring-boot/docs/current/reference/html/howto.html#howto.data-initialization.migration-tool.flyway)
- [PostgreSQL Documentation](https://www.postgresql.org/docs/)

## Support

For questions or issues related to database migrations, please contact the Backend Team or open an issue in the project repository.

