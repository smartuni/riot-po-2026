-- Migration: V8__Fix_status_column.sql
-- Description: Convert gates.status from status_enum to VARCHAR to match Hibernate's
-- @Enumerated(EnumType.STRING) mapping. Without this, inserts via the GUI
-- (GateService.addGateFromGUI) fail with:
--   column "status" is of type status_enum but expression is of type character varying
-- This completes the conversion started for quality (V4) and state_confirmation (V5).
--
-- The v_gate_summary and v_recent_activities views read gates.status, so they must
-- be dropped before the column type change and recreated afterwards (Postgres rejects
-- altering a column type while a view depends on it).

DROP VIEW IF EXISTS v_recent_activities;
DROP VIEW IF EXISTS v_gate_summary;

ALTER TABLE gates ALTER COLUMN status TYPE VARCHAR(50)
    USING status::text;

CREATE OR REPLACE VIEW v_gate_summary AS
SELECT
    id,
    status,
    location,
    latitude,
    longitude,
    confidence,
    priority,
    gate_time_stamp,
    hlc_log,
    created_at
FROM gates
WHERE ignore_gate = FALSE
ORDER BY priority DESC, last_time_stamp DESC,hlc_log DESC;

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
