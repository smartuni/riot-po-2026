-- Migration: V12__Add_device_keys.sql
-- Description: Add Ed25519 key storage tables for asymmetric signing
--              (mirrors firmware credential_manager pattern)

CREATE TABLE device_public_keys (
    id BIGSERIAL PRIMARY KEY,
    kid VARCHAR(255) NOT NULL UNIQUE,
    public_key BYTEA NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

COMMENT ON TABLE device_public_keys IS 'Ed25519 public keys of devices (gates, mates) for signature verification';
COMMENT ON COLUMN device_public_keys.kid IS 'Key ID (device name, e.g. sensegate-1, sensemate-2)';
COMMENT ON COLUMN device_public_keys.public_key IS '32-byte Ed25519 public key';

CREATE INDEX idx_device_public_keys_kid ON device_public_keys(kid);

CREATE TABLE server_key (
    id BIGINT PRIMARY KEY DEFAULT 1 CHECK (id = 1),
    kid VARCHAR(255) NOT NULL DEFAULT 'server',
    private_key BYTEA NOT NULL,
    public_key BYTEA NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

COMMENT ON TABLE server_key IS 'Singleton table storing the backend server own Ed25519 key pair';
COMMENT ON COLUMN server_key.kid IS 'Key ID for the server (used as writer ID in signed records)';
COMMENT ON COLUMN server_key.private_key IS '64-byte Ed25519 private key seed';
COMMENT ON COLUMN server_key.public_key IS '32-byte Ed25519 public key';
