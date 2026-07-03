-- Migration: V12__Add_kid_and_triggers.sql
-- Description: Add kid column and unique constraint to root_keys, add updated_at triggers for new tables

-- ============================================================================
-- root_keys: Add kid column and enforce singleton constraint
-- ============================================================================
ALTER TABLE root_keys ADD COLUMN IF NOT EXISTS kid VARCHAR(255);

-- Backfill existing rows with default kid
UPDATE root_keys SET kid = 'server' WHERE kid IS NULL;

ALTER TABLE root_keys ALTER COLUMN kid SET NOT NULL;

-- Enforce singleton: only one root key row allowed
CREATE UNIQUE INDEX IF NOT EXISTS idx_root_keys_singleton ON root_keys ((1));

-- ============================================================================
-- updated_at triggers for new tables (follows V6 convention)
-- ============================================================================
CREATE TRIGGER update_root_keys_updated_at BEFORE UPDATE ON root_keys
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_nodes_updated_at BEFORE UPDATE ON nodes
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();
