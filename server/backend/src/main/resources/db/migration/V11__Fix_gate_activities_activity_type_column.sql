-- Migration: V11__Fix_gate_activities_activity_type_column.sql
-- Description: Convert gate_activities.activity_type from activity_type_enum to VARCHAR
-- to match Hibernate's @Enumerated(EnumType.STRING) mapping. Without this, inserts via
-- the entity (gateActivityService.addGateActivity -> repository.save) fail with:
--   column "activity_type" is of type activity_type_enum but expression is of type character varying
-- Same fix as gates.status (V8) and notifications.status (V9).
--
-- The v_recent_activities view selects ga.activity_type, and v_gate_summary depends
-- on gates which was recreated in V8, so both must be dropped before the column type
-- change and recreated afterwards.

DROP VIEW IF EXISTS v_recent_activities;
DROP VIEW IF EXISTS v_gate_summary;

ALTER TABLE gate_activities ALTER COLUMN activity_type TYPE VARCHAR(50)
    USING activity_type::text;

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
