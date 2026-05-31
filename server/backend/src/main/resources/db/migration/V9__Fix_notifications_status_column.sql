-- Migration: V9__Fix_notifications_status_column.sql
-- Description: Convert notifications.status from status_enum to VARCHAR to match
-- Hibernate's @Enumerated(EnumType.STRING) mapping. Without this, creating a
-- notification (NotificationService.addNotification -> repository.save) fails with:
--   column "status" is of type status_enum but expression is of type character varying
-- Same fix as gates.status (V8). No view depends on this column.
ALTER TABLE notifications ALTER COLUMN status TYPE VARCHAR(50)
    USING status::text;
