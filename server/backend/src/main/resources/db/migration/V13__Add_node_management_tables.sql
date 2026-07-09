-- Migration: V11__Add_node_management_tables.sql
-- Description: Create tables for node management (root keys and nodes)

-- ============================================================================
-- Table: root_keys
-- Description: Stores Ed25519 root key pair for node authentication
-- ============================================================================
CREATE TABLE root_keys (
    id BIGSERIAL PRIMARY KEY,
    public_key TEXT NOT NULL,
    private_key TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- Table: nodes
-- Description: Stores registered nodes and their Ed25519 public keys
-- ============================================================================
CREATE TABLE nodes (
    id BIGSERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    public_key TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_nodes_name ON nodes(name);
