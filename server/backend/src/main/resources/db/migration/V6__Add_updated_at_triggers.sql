-- Migration: V6__Add_updated_at_triggers.sql
-- Description: Add triggers to automatically update the updated_at timestamp on UPDATE operations
-- Author: Backend Team
-- Date: 2026-05-18

-- Create a generic function for updating the updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Create trigger for users table
CREATE TRIGGER update_users_updated_at BEFORE UPDATE ON users
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Create trigger for gates table
CREATE TRIGGER update_gates_updated_at BEFORE UPDATE ON gates
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Create trigger for notifications table
CREATE TRIGGER update_notifications_updated_at BEFORE UPDATE ON notifications
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Commit message: feat: #5 add automatic updated_at triggers for audit trail
-- - Create update_updated_at_column() function for all tables
-- - Add triggers on users, gates, notifications tables
-- - Ensures updated_at is automatically set on every UPDATE operation

