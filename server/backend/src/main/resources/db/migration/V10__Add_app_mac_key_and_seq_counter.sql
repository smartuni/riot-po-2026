-- Migration: V8__Add_app_mac_key_and_seq_counter.sql
-- Description: Add app_mac_key and last_seq_tx columns to gates table for
--              AES-CMAC downlink signing (see ticket-64-downlink-cmac)

ALTER TABLE gates
    ADD COLUMN app_mac_key BYTEA,
    ADD COLUMN last_seq_tx SMALLINT NOT NULL DEFAULT 0;

COMMENT ON COLUMN gates.app_mac_key IS '16-byte AES-CMAC key (AppMACKey) provisioned per device at flash time';
COMMENT ON COLUMN gates.last_seq_tx IS 'Last transmitted sequence counter for replay protection (wraps at 0xFFFF)';
