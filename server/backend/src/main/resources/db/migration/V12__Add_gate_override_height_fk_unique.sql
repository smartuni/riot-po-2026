-- V12: Add gate override/height columns, FK cascade on gate_metadata, unique constraint
-- Issue #1: GateEntity.java has manualOverride (boolean) and heightAboveNN (Double) fields
--           but no migration adds these columns to the gates table in PostgreSQL.
-- Issue #2: gate_metadata.gate_id has no FK to gates(id) with ON DELETE CASCADE.
-- Issue #10: No UNIQUE(gate_id, key) constraint on gate_metadata.

-- Issue #1: Add columns matching GateEntity.java field annotations
-- GateEntity.java: private boolean manualOverride; (no @Column → maps to "manual_override")
-- GateEntity.java: @Column(name = "height_above_nn") private Double heightAboveNN;
ALTER TABLE gates ADD COLUMN IF NOT EXISTS manual_override BOOLEAN DEFAULT FALSE;
ALTER TABLE gates ADD COLUMN IF NOT EXISTS height_above_nn DOUBLE PRECISION;

-- Issue #2: Add FK with ON DELETE CASCADE (following the V7__Add_foreign_key_constraints.sql pattern)
-- First clean up any orphaned metadata rows (gate_id doesn't exist in gates)
DELETE FROM gate_metadata WHERE gate_id NOT IN (SELECT id FROM gates);

ALTER TABLE gate_metadata
    DROP CONSTRAINT IF EXISTS fk_gate_metadata_gate_id;

ALTER TABLE gate_metadata
    ADD CONSTRAINT fk_gate_metadata_gate_id
    FOREIGN KEY (gate_id) REFERENCES gates(id) ON DELETE CASCADE;

-- Issue #10: Add UNIQUE(gate_id, key) constraint to prevent duplicate metadata entries
-- First deduplicate existing rows: keep the latest (max id) per (gate_id, key), delete the rest
DELETE FROM gate_metadata
WHERE id NOT IN (
    SELECT MAX(id) FROM gate_metadata
    GROUP BY gate_id, "key"
);

ALTER TABLE gate_metadata
    DROP CONSTRAINT IF EXISTS uk_gate_metadata_gate_id_key;

ALTER TABLE gate_metadata
    ADD CONSTRAINT uk_gate_metadata_gate_id_key
    UNIQUE (gate_id, "key");
