-- Migration: V1__Initial_schema.sql
-- Description: Create initial database schema for Riot-PO application
-- Author: Backend Team
-- Date: 2026-04-27

-- Create ENUM types
CREATE TYPE status_enum AS ENUM ('OPEN', 'CLOSED', 'OUT_OF_SERVICE', 'NONE');
CREATE TYPE state_confirmation_enum AS ENUM ('CONFIRMED', 'UNCONFIRMED');
CREATE TYPE confidence_quality_enum AS ENUM ('HIGH', 'MEDIUM', 'LOW');
CREATE TYPE activity_type_enum AS ENUM (
    'SENSOR_NEW',
    'SENSOR_VALUE_CHANGED',
    'SENSOR_VALUE_KEEPALIVE',
    'SENSEMATE_WORKER_REPORT',
    'TARGET_STATE_REQUEST'
);

-- ============================================================================
-- Table: users
-- Description: Stores user information (email, password, name, role)
-- ============================================================================
CREATE TABLE users (
    id BIGSERIAL PRIMARY KEY,
    email VARCHAR(255) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    name VARCHAR(255),
    role VARCHAR(50),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_users_role ON users(role);

-- ============================================================================
-- Table: gates
-- Description: Stores gate/sensor information and their current state
-- ============================================================================
CREATE TABLE gates (
    id BIGINT PRIMARY KEY,
    status status_enum DEFAULT 'NONE',
    state_confirmation state_confirmation_enum DEFAULT 'UNCONFIRMED',
    last_time_stamp TIMESTAMP,
    last_transition_gate_time_stamp TIMESTAMP,
    device_id BIGINT,
    location VARCHAR(255),
    latitude DOUBLE PRECISION,
    longitude DOUBLE PRECISION,
    worker_confidence BOOLEAN,
    sensor_confidence BOOLEAN,
    quality confidence_quality_enum,
    ignore_gate BOOLEAN DEFAULT FALSE,
    gate_detector BOOLEAN DEFAULT FALSE,
    gate_status_array status_enum[] DEFAULT ARRAY['NONE'::status_enum, 'NONE'::status_enum, 'NONE'::status_enum],
    worker_status_array status_enum[] DEFAULT ARRAY['NONE'::status_enum, 'NONE'::status_enum, 'NONE'::status_enum],
    requested_status VARCHAR(50),
    pending_job VARCHAR(255),
    confidence INTEGER DEFAULT 0,
    priority INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_gates_device_id ON gates(device_id);
CREATE INDEX idx_gates_location ON gates(location);
CREATE INDEX idx_gates_status ON gates(status);
CREATE INDEX idx_gates_created_at ON gates(created_at);

-- ============================================================================
-- Table: gate_activities
-- Description: Audit log for all gate state changes and worker actions
-- ============================================================================
CREATE TABLE gate_activities (
    id BIGSERIAL PRIMARY KEY,
    last_time_stamp TIMESTAMP,
    local_time_stamp TIMESTAMP,
    gate_time_stamp TIMESTAMP,
    gate_id BIGINT NOT NULL,
    requested_status VARCHAR(50),
    message TEXT,
    worker_id BIGINT,
    activity_type activity_type_enum NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_gate_activities_gate_id ON gate_activities(gate_id);
CREATE INDEX idx_gate_activities_worker_id ON gate_activities(worker_id);
CREATE INDEX idx_gate_activities_activity_type ON gate_activities(activity_type);
CREATE INDEX idx_gate_activities_created_at ON gate_activities(created_at);

-- Foreign key constraint (optional, depending on data integrity needs)
-- ALTER TABLE gate_activities ADD CONSTRAINT fk_gate_activities_gate_id
--     FOREIGN KEY (gate_id) REFERENCES gates(id) ON DELETE CASCADE;

-- ============================================================================
-- Table: notifications
-- Description: Stores notifications for users about gate events
-- ============================================================================
CREATE TABLE notifications (
    id BIGSERIAL PRIMARY KEY,
    status status_enum,
    last_time_stamp TIMESTAMP,
    worker_id BIGINT,
    message TEXT,
    read BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_notifications_worker_id ON notifications(worker_id);
CREATE INDEX idx_notifications_read ON notifications(read);
CREATE INDEX idx_notifications_created_at ON notifications(created_at);

-- ============================================================================
-- Table: gate_for_downlink
-- Description: Stores gates queued for downlink operations
-- ============================================================================
CREATE TABLE gate_for_downlink (
    id BIGSERIAL PRIMARY KEY,
    gate_id BIGINT NOT NULL,
    requested_status VARCHAR(50),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_gate_for_downlink_gate_id ON gate_for_downlink(gate_id);

-- ============================================================================
-- Table: downlink_counter
-- Description: Tracks downlink operations counter for monitoring
-- ============================================================================
CREATE TABLE downlink_counter (
    id BIGSERIAL PRIMARY KEY,
    counter INTEGER DEFAULT 0,
    last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- Initial seed data (optional)
-- ============================================================================
-- Insert a default downlink counter
INSERT INTO downlink_counter (counter) VALUES (0);

-- Insert test users (optional - remove in production)
INSERT INTO users (email, password, name, role)
VALUES
    ('test@example.com', '$2a$10$slYQmyNdGzin7olVN3p5Be7DYC0tgO7XdClpKnJmH8E7DvN3hTfVe', 'Test User', 'controller'),
    ('viewer@example.com', '$2a$10$slYQmyNdGzin7olVN3p5Be7DYC0tgO7XdClpKnJmH8E7DvN3hTfVe', 'Viewer User', 'viewer')
ON CONFLICT (email) DO NOTHING;

-- ============================================================================
-- Create views (optional - for common queries)
-- ============================================================================
CREATE OR REPLACE VIEW v_gate_summary AS
SELECT
    id,
    status,
    location,
    latitude,
    longitude,
    confidence,
    priority,
    last_time_stamp,
    created_at
FROM gates
WHERE ignore_gate = FALSE
ORDER BY priority DESC, last_time_stamp DESC;

CREATE OR REPLACE VIEW v_recent_activities AS
SELECT
    ga.id,
    ga.gate_id,
    ga.message,
    ga.activity_type,
    ga.worker_id,
    ga.created_at,
    g.location,
    g.status
FROM gate_activities ga
LEFT JOIN gates g ON ga.gate_id = g.id
ORDER BY ga.created_at DESC
LIMIT 100;

-- Commit message for this migration:
-- feat: #4 add initial PostgreSQL database schema with Flyway migration
-- - Create ENUM types for status, state_confirmation, confidence_quality, activity_type
-- - Create tables: users, gates, gate_activities, notifications, gate_for_downlink, downlink_counter
-- - Add indexes for performance optimization
-- - Create views for common queries
-- - Insert default seed data for testing

