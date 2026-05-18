-- Migration: V7__Add_foreign_key_constraints.sql
-- Description: Enable foreign key constraints for referential integrity
-- Author: Backend Team
-- Date: 2026-05-18

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

-- Commit message: feat: #6 enable foreign key constraints for referential integrity
-- - Add FK constraint on gate_activities -> gates (CASCADE DELETE)
-- - Add FK constraint on gate_activities -> users (SET NULL)
-- - Add FK constraint on gate_for_downlink -> gates (CASCADE Delete)
-- - Add FK constraint on notifications -> users (SET NULL)
-- - Ensures database integrity and prevents orphaned records

