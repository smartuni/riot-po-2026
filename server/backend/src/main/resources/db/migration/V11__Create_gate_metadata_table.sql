-- Migration: V11__Create_gate_metadata_table.sql
-- Description: Create gate_metadata table for persistent key-value metadata on gates
-- Author: Backend Team
-- Date: 2026-07-04
-- Related issue: #79

-- ============================================================================
-- Table: gate_metadata
-- Description: Stores persistent key-value metadata for gates
-- ============================================================================
CREATE TABLE gate_metadata (
    id BIGSERIAL PRIMARY KEY,
    gate_id BIGINT NOT NULL,
    "key" VARCHAR(255) NOT NULL,
    "value" TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_gate_metadata_gate_id ON gate_metadata(gate_id);
CREATE INDEX idx_gate_metadata_key ON gate_metadata("key");
