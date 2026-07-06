-- Migration: V2__Insert_seed_data.sql
-- Description: Insert seed/test data for development and testing
-- Author: Backend Team
-- Date: 2026-04-27

-- Users are managed by AuthService startup logic and application properties.
-- DO NOT insert them here.

-- Insert sample gates for testing
INSERT INTO gates (id, status, state_confirmation, gate_time_stamp,hlc_log, device_id, location, latitude, longitude, requested_status, confidence, priority, created_at, updated_at) VALUES
(1, 'OPEN'::status_enum, 'CONFIRMED'::state_confirmation_enum, CURRENT_TIMESTAMP,1, 101, 'Hauptbahnhof', 53.5400, 9.8700, 'OPEN', 85, 3, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP),
(2, 'OPEN'::status_enum, 'CONFIRMED'::state_confirmation_enum, CURRENT_TIMESTAMP,0, 102, 'Landungsbrücken', 53.5409, 9.8674, 'OPEN', 90, 2, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP),
(3, 'CLOSED'::status_enum, 'CONFIRMED'::state_confirmation_enum, CURRENT_TIMESTAMP, 3,103, 'Veddel', 53.5410, 9.8664, 'CLOSED', 92, 1, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP),
(4, 'OPEN'::status_enum, 'UNCONFIRMED'::state_confirmation_enum, CURRENT_TIMESTAMP, 4,104, 'Hafen', 53.5430, 9.8434, 'CLOSED', 75, 2, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP),
(5, 'CLOSED'::status_enum, 'CONFIRMED'::state_confirmation_enum, CURRENT_TIMESTAMP, 0,105, 'Steinwerder', 53.5440, 9.8734, 'CLOSED', 88, 1, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP),
(6, 'OPEN'::status_enum, 'CONFIRMED'::state_confirmation_enum, CURRENT_TIMESTAMP, 2,106, 'Neumühlen', 53.5470, 9.8634, 'OPEN', 91, 2, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
ON CONFLICT (id) DO NOTHING;

-- Insert sample notifications for testing
-- NOTE: Only use worker_ids that exist in users table (1 and 2)
-- or NULL for system-generated notifications
INSERT INTO notifications (status, worker_id, message, read, created_at, updated_at) VALUES
('OPEN'::status_enum, 1, 'Worker with ID: 1 shall close the Gate with ID: 1', false, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP),
('OPEN'::status_enum, 2, 'Worker with ID: 2 shall close the Gate with ID: 1', false, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP),
('CLOSED'::status_enum, NULL, 'Gate 3 has been closed successfully', true, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP),
('OPEN'::status_enum, NULL, 'System notification: verify Gate with ID: 4', false, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP),
('CLOSED'::status_enum, NULL, 'All gates secured', true, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP);

-- Insert sample gate activities (audit log)
-- NOTE: These are sensor-initiated events, so worker_id is NULL
INSERT INTO gate_activities (local_time_stamp, gate_time_stamp,hlc_log, gate_id, requested_status, message, worker_id, activity_type, created_at) VALUES
( CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, 2,1, 'OPEN', 'Gate 1 reported status OPEN', NULL, 'SENSOR_VALUE_KEEPALIVE', CURRENT_TIMESTAMP),
( CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, 3,2, 'OPEN', 'Gate 2 reported status OPEN', NULL, 'SENSOR_VALUE_KEEPALIVE', CURRENT_TIMESTAMP),
( CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, 0,3, 'CLOSED', 'Gate 3 has been closed', NULL, 'SENSEMATE_WORKER_REPORT', CURRENT_TIMESTAMP),
( CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, 2,4, 'CLOSED', 'Worker requested status CLOSED for Gate 4', NULL, 'TARGET_STATE_REQUEST', CURRENT_TIMESTAMP);

-- Note: CommandLineRunner in Application.java is now conditional:
-- - @Profile("dev") ensures it only runs when spring.profiles.active=dev
-- - Test data is seeded via this SQL script during Flyway migration
-- - Production deployments should NOT use the dev profile

