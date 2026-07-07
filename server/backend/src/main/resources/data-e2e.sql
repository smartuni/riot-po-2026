-- E2E seed data for H2 (compatible syntax, no PostgreSQL-specific casts)
-- Tables are created fresh by Hibernate DDL auto (create-drop),
-- so no truncate statements are needed.

-- Seed the test accounts with fixed IDs for deterministic worker_id references.
-- Passwords are BCrypt hashes of test-credentials from application.yml:
--   test@example.com  / test123  (controller)
--   test2@example.com / test234  (viewer)
-- AuthService skips re-creating them on startup since they already exist.
-- Users are created by AuthService (test-credentials from application.yml)
-- with auto-generated IDs 1 and 2. The seed data below references worker_id
-- values 1 and 2 which match those auto-generated IDs.

INSERT INTO gates (
    id, status, state_confirmation, gate_time_stamp, last_transition_gate_time_stamp,
    device_id, location, latitude, longitude, requested_status, confidence, priority,
    ignore_gate, gate_detector, manual_override, height_above_nn
) VALUES
    (1001, 'OPEN', 'WORKER_CONFIRMED_SINGLE',
      TIMESTAMP '2026-01-01 08:00:00', TIMESTAMP '2026-01-01 07:55:00',
      501, 'E2E Gate Alpha', 53.5500, 9.9937, 'OPEN', 90, 3,
      false, false, false, 2.5),
    (1002, 'CLOSED', 'WORKER_CONFIRMED_MULTI',
      TIMESTAMP '2026-01-01 08:05:00', TIMESTAMP '2026-01-01 08:00:00',
      502, 'E2E Gate Beta', 53.5510, 9.9940, 'CLOSED', 85, 2,
      false, false, false, 3.8),
    (1003, 'OPEN', 'UNCONFIRMED',
      TIMESTAMP '2026-01-01 08:10:00', TIMESTAMP '2026-01-01 08:10:00',
      503, 'E2E Gate Gamma', 53.5520, 9.9950, 'OPEN', 70, 1,
      false, false, false, 1.2),
    (1004, 'OUT_OF_SERVICE', 'WORKER_CONFLICT',
      TIMESTAMP '2026-01-01 08:15:00', TIMESTAMP '2026-01-01 08:15:00',
      504, 'E2E Gate Delta', 53.5530, 9.9960, 'OUT_OF_SERVICE', 40, 0,
      false, false, false, 5.0);

INSERT INTO gate_metadata (gate_id, "key", "value") VALUES
    (1001, 'closing_threshold_cm', '250'),
    (1001, 'max_water_flow', '12.5'),
    (1002, 'closing_threshold_cm', '380'),
    (1002, 'last_inspection', '2026-01-15'),
    (1003, 'closing_threshold_cm', '120');

INSERT INTO notifications (
    status, last_time_stamp, worker_id, message, read
) VALUES
    ('OPEN', TIMESTAMP '2026-01-01 08:20:00', 1,
     'Worker 1 should verify Gate 1001', false),
    ('CLOSED', TIMESTAMP '2026-01-01 08:25:00', 2,
     'Worker 2 should close Gate 1002', false);

INSERT INTO gate_activities (
    last_time_stamp, local_time_stamp, gate_time_stamp, gate_id,
    requested_status, message, worker_id, activity_type
) VALUES
    (TIMESTAMP '2026-01-01 08:00:00', TIMESTAMP '2026-01-01 08:00:00', TIMESTAMP '2026-01-01 08:00:00',
     1001, 'OPEN', 'E2E seed: Gate 1001 OPEN', NULL, 'SENSOR_VALUE_KEEPALIVE'),
    (TIMESTAMP '2026-01-01 08:05:00', TIMESTAMP '2026-01-01 08:05:00', TIMESTAMP '2026-01-01 08:05:00',
     1002, 'CLOSED', 'E2E seed: Gate 1002 CLOSED', NULL, 'SENSOR_VALUE_KEEPALIVE');

INSERT INTO downlink_counter (id, counter) VALUES (1, 0);
