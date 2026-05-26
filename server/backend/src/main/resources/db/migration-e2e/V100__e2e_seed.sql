-- E2E seed data for deterministic test runs

-- Start clean within a fresh database
TRUNCATE TABLE gate_activities RESTART IDENTITY CASCADE;
TRUNCATE TABLE notifications RESTART IDENTITY CASCADE;
TRUNCATE TABLE gates RESTART IDENTITY CASCADE;

INSERT INTO gates (
    id, status, state_confirmation, last_time_stamp, last_transition_gate_time_stamp,
    device_id, location, latitude, longitude, requested_status, confidence, priority,
    created_at, updated_at
) VALUES
    (1001, 'OPEN'::status_enum, 'CONFIRMED'::state_confirmation_enum,
        TIMESTAMP '2026-01-01 08:00:00', TIMESTAMP '2026-01-01 07:55:00',
        501, 'E2E Gate Alpha', 53.5500, 9.9937, 'OPEN', 90, 3,
        TIMESTAMP '2026-01-01 08:00:00', TIMESTAMP '2026-01-01 08:00:00'),
    (1002, 'CLOSED'::status_enum, 'CONFIRMED'::state_confirmation_enum,
        TIMESTAMP '2026-01-01 08:05:00', TIMESTAMP '2026-01-01 08:00:00',
        502, 'E2E Gate Beta', 53.5510, 9.9940, 'CLOSED', 85, 2,
        TIMESTAMP '2026-01-01 08:05:00', TIMESTAMP '2026-01-01 08:05:00'),
    (1003, 'OPEN'::status_enum, 'UNCONFIRMED'::state_confirmation_enum,
        TIMESTAMP '2026-01-01 08:10:00', TIMESTAMP '2026-01-01 08:10:00',
        503, 'E2E Gate Gamma', 53.5520, 9.9950, 'OPEN', 70, 1,
        TIMESTAMP '2026-01-01 08:10:00', TIMESTAMP '2026-01-01 08:10:00'),
    (1004, 'OUT_OF_SERVICE'::status_enum, 'UNCONFIRMED'::state_confirmation_enum,
        TIMESTAMP '2026-01-01 08:15:00', TIMESTAMP '2026-01-01 08:15:00',
        504, 'E2E Gate Delta', 53.5530, 9.9960, 'OUT_OF_SERVICE', 40, 0,
        TIMESTAMP '2026-01-01 08:15:00', TIMESTAMP '2026-01-01 08:15:00');

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
    last_time_stamp, local_time_stamp, gate_time_stamp, gate_id,
    requested_status, message, worker_id, activity_type, created_at
) VALUES
    (TIMESTAMP '2026-01-01 08:00:00', TIMESTAMP '2026-01-01 08:00:00', TIMESTAMP '2026-01-01 08:00:00',
        1001, 'OPEN', 'E2E seed: Gate 1001 OPEN', NULL, 'SENSOR_VALUE_KEEPALIVE'::activity_type_enum,
        TIMESTAMP '2026-01-01 08:00:00'),
    (TIMESTAMP '2026-01-01 08:05:00', TIMESTAMP '2026-01-01 08:05:00', TIMESTAMP '2026-01-01 08:05:00',
        1002, 'CLOSED', 'E2E seed: Gate 1002 CLOSED', NULL, 'SENSOR_VALUE_KEEPALIVE'::activity_type_enum,
        TIMESTAMP '2026-01-01 08:05:00');

