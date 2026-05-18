-- Migration: V4__Fix_quality_enum_column.sql
-- Description: Convert confidence_quality_enum column to VARCHAR to match Hibernate's @Enumerated(EnumType.STRING) mapping

ALTER TABLE gates ALTER COLUMN quality TYPE VARCHAR(50)
    USING quality::text;
