-- Migration: V3__Fix_enum_array_columns.sql
-- Description: Convert status_enum[] columns to smallint[] to match Hibernate's default ordinal mapping

-- Drop defaults first so column type can be changed
ALTER TABLE gates ALTER COLUMN gate_status_array DROP DEFAULT;
ALTER TABLE gates ALTER COLUMN worker_status_array DROP DEFAULT;

-- Create conversion function
CREATE OR REPLACE FUNCTION status_enum_to_smallint(status_enum[])
RETURNS smallint[] AS $$
DECLARE
    result smallint[];
    i integer;
BEGIN
    FOR i IN 1..array_length($1, 1) LOOP
        result[i] := CASE $1[i]
            WHEN 'OPEN'::status_enum THEN 0::smallint
            WHEN 'CLOSED'::status_enum THEN 1::smallint
            WHEN 'OUT_OF_SERVICE'::status_enum THEN 2::smallint
            WHEN 'NONE'::status_enum THEN 3::smallint
            ELSE 3::smallint
        END;
    END LOOP;
    RETURN result;
END;
$$ LANGUAGE plpgsql;

-- Alter columns using the conversion function
ALTER TABLE gates ALTER COLUMN gate_status_array TYPE smallint[]
    USING status_enum_to_smallint(gate_status_array);

ALTER TABLE gates ALTER COLUMN worker_status_array TYPE smallint[]
    USING status_enum_to_smallint(worker_status_array);

-- Add back defaults with new type
ALTER TABLE gates ALTER COLUMN gate_status_array SET DEFAULT ARRAY[3::smallint, 3::smallint, 3::smallint];
ALTER TABLE gates ALTER COLUMN worker_status_array SET DEFAULT ARRAY[3::smallint, 3::smallint, 3::smallint];

-- Drop the conversion function
DROP FUNCTION status_enum_to_smallint(status_enum[]);
