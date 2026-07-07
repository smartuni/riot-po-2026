-- E2E seed data for deterministic test runs

-- Start clean within a fresh database
TRUNCATE TABLE gate_activities RESTART IDENTITY CASCADE;
TRUNCATE TABLE notifications RESTART IDENTITY CASCADE;
TRUNCATE TABLE gates RESTART IDENTITY CASCADE;
TRUNCATE TABLE users RESTART IDENTITY CASCADE;
TRUNCATE TABLE gate_metadata RESTART IDENTITY CASCADE;

-- Seed the test accounts here (not only via application.yml) for two reasons:
--   1. The notifications below reference users via fk_notifications_worker_id,
--      and Flyway runs before the app's AuthService creates the test accounts.
--   2. Fixed ids make worker_id linkage deterministic. UserDetailsResponse.workerId
--      equals users.id, so worker_id 1/2 below belong to the accounts below.
-- Passwords are BCrypt hashes of the application.yml test-credentials:
--   test@example.com  / test123  (controller)
--   test2@example.com / test234  (viewer)
-- AuthService finds these on startup and skips re-creating them, so login uses
-- exactly these hashes.
INSERT INTO users (id, email, password, name, role, created_at, updated_at) VALUES
    (1, 'test@example.com',
        '$2a$10$LI97iwxKfBMUlwjxEuG.3e1.3/3GCkBWvUXb6dpc9fS6zsTfoEyTC',
        'test', 'controller',
        TIMESTAMP '2026-01-01 08:00:00', TIMESTAMP '2026-01-01 08:00:00'),
    (2, 'test2@example.com',
        '$2a$10$CFf1kL3wvS8pwaF0Can1l.2dRmzftfXbgkyNPMMwX2JHEmZnw5C4G',
        'test2', 'viewer',
        TIMESTAMP '2026-01-01 08:00:00', TIMESTAMP '2026-01-01 08:00:00');

-- Advance the sequence so later registrations do not collide with the fixed ids.
SELECT setval('users_id_seq', (SELECT MAX(id) FROM users));

INSERT INTO gates (
    id, status, state_confirmation, gate_time_stamp, hlc_log,last_transition_gate_time_stamp,
    device_id, location, latitude, longitude, requested_status, confidence, priority,
    height_above_nn, created_at, updated_at
) VALUES
    (1001, 'OPEN'::status_enum, 'CONFIRMED'::state_confirmation_enum,
        TIMESTAMP '2026-01-01 08:00:00', 0,TIMESTAMP '2026-01-01 07:55:00',
        501, 'E2E Gate Alpha', 53.5500, 9.9937, 'OPEN', 90, 3,2.5,
        TIMESTAMP '2026-01-01 08:00:00', TIMESTAMP '2026-01-01 08:00:00'),
    (1002, 'CLOSED'::status_enum, 'CONFIRMED'::state_confirmation_enum,
        TIMESTAMP '2026-01-01 08:05:00', 1,TIMESTAMP '2026-01-01 08:00:00',
        502, 'E2E Gate Beta', 53.5510, 9.9940, 'CLOSED', 85, 2,3.8
        TIMESTAMP '2026-01-01 08:05:00', TIMESTAMP '2026-01-01 08:05:00'),
    (1003, 'OPEN'::status_enum, 'UNCONFIRMED'::state_confirmation_enum,
        TIMESTAMP '2026-01-01 08:10:00',2, TIMESTAMP '2026-01-01 08:10:00',
        503, 'E2E Gate Gamma', 53.5520, 9.9950, 'OPEN', 70, 1,1.2
        TIMESTAMP '2026-01-01 08:10:00', TIMESTAMP '2026-01-01 08:10:00'),
    (1004, 'OUT_OF_SERVICE'::status_enum, 'UNCONFIRMED'::state_confirmation_enum,
        TIMESTAMP '2026-01-01 08:15:00', 3,TIMESTAMP '2026-01-01 08:15:00',
        504, 'E2E Gate Delta', 53.5530, 9.9960, 'OUT_OF_SERVICE', 40, 0,5.0
        TIMESTAMP '2026-01-01 08:15:00', TIMESTAMP '2026-01-01 08:15:00');

INSERT INTO gate_metadata (gate_id, "key", "value") VALUES
    (1001, 'closing_threshold_cm', '250'),
    (1001, 'max_water_flow', '12.5'),
    (1002, 'closing_threshold_cm', '380'),
    (1002, 'last_inspection', '2026-01-15'),
    (1003, 'closing_threshold_cm', '120');

INSERT INTO notifications (
    status, last_time_stamp, worker_id, message, read, created_at, updated_at
) VALUES
    ('OPEN'::status_enum, TIMESTAMP '2026-01-01 08:20:00', 1,
        'Worker 1 should verify Gate 1001', false,
        TIMESTAMP '2026-01-01 08:20:00', TIMESTAMP '2026-01-01 08:20:00'),
    ('CLOSED'::status_enum, TIMESTAMP '2026-01-01 08:25:00', 2,
        'Worker 2 should close Gate 1002', false,
        TIMESTAMP '2026-01-01 08:25:00', TIMESTAMP '2026-01-01 08:25:00');

INSERT INTO gate_activities (
    local_time_stamp, gate_time_stamp,hlc_log, gate_id,
    requested_status, message, worker_id, activity_type, created_at
) VALUES
    (TIMESTAMP '2026-01-01 08:00:00', TIMESTAMP '2026-01-01 08:00:00',
        2,1001, 'OPEN', 'E2E seed: Gate 1001 OPEN', NULL, 'SENSOR_VALUE_KEEPALIVE',
        TIMESTAMP '2026-01-01 08:00:00'),
    ( TIMESTAMP '2026-01-01 08:05:00', TIMESTAMP '2026-01-01 08:05:00',
        4,1002, 'CLOSED', 'E2E seed: Gate 1002 CLOSED', NULL, 'SENSOR_VALUE_KEEPALIVE',
        TIMESTAMP '2026-01-01 08:05:00');

