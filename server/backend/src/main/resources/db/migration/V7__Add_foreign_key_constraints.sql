-- Migration: V7__Add_foreign_key_constraints.sql
-- Description: Enable foreign key constraints for referential integrity
-- Author: Backend Team
-- Date: 2026-05-18

-- Step 1: Clean up invalid foreign key references before adding constraints
-- Set worker_id to NULL for notifications where worker doesn't exist
UPDATE notifications
SET worker_id = NULL
WHERE worker_id IS NOT NULL
  AND worker_id NOT IN (SELECT id FROM users);

-- Set worker_id to NULL for gate_activities where worker doesn't exist
UPDATE gate_activities
SET worker_id = NULL
WHERE worker_id IS NOT NULL
  AND worker_id NOT IN (SELECT id FROM users);

-- Remove rows from gate_for_downlink that reference non-existent gates
DELETE FROM gate_for_downlink
WHERE gate_id NOT IN (SELECT id FROM gates);

-- Remove rows from gate_activities that reference non-existent gates
DELETE FROM gate_activities
WHERE gate_id NOT IN (SELECT id FROM gates);

-- Step 2: Add foreign key constraints for referential integrity

-- Enable foreign key constraint for gate_activities -> gates
ALTER TABLE gate_activities
ADD CONSTRAINT fk_gate_activities_gate_id
FOREIGN KEY (gate_id) REFERENCES gates(id) ON DELETE CASCADE;

-- Add optional foreign key for gate_activities -> users (worker_id)
-- Note: This is optional since worker_id can be NULL for sensor-initiated events
ALTER TABLE gate_activities
ADD CONSTRAINT fk_gate_activities_worker_id
FOREIGN KEY (worker_id) REFERENCES users(id) ON DELETE SET NULL;

-- Add optional foreign key for gate_for_downlink -> gates
ALTER TABLE gate_for_downlink
ADD CONSTRAINT fk_gate_for_downlink_gate_id
FOREIGN KEY (gate_id) REFERENCES gates(id) ON DELETE CASCADE;

-- Add optional foreign key for notifications -> users (worker_id)
ALTER TABLE notifications
ADD CONSTRAINT fk_notifications_worker_id
FOREIGN KEY (worker_id) REFERENCES users(id) ON DELETE SET NULL;

-- Commit message: feat: #7 enable foreign key constraints with data cleanup
-- - Clean up invalid references before adding FK constraints
-- - Set NULL for invalid worker_ids in notifications and gate_activities
-- - Delete orphaned records from gate_for_downlink and gate_activities
-- - Add FK constraint on gate_activities -> gates (CASCADE Delete)
-- - Add FK constraint on gate_activities -> users (SET NULL)
-- - Add FK constraint on gate_for_downlink -> gates (CASCADE Delete)
-- - Add FK constraint on notifications -> users (SET NULL)
-- - Ensures database integrity and prevents orphaned records

