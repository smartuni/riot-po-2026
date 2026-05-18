-- Migration: V5__Fix_state_confirmation_column.sql
-- Description: Convert state_confirmation column to VARCHAR to match Hibernate's @Enumerated(EnumType.STRING) mapping and support all business logic enum values
ALTER TABLE gates ALTER COLUMN state_confirmation TYPE VARCHAR(50)
    USING state_confirmation::text;
