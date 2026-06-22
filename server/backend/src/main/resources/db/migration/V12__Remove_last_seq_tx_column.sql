-- Migration: V12__Remove_last_seq_tx_column.sql
-- Description: Remove last_seq_tx column from gates table.
--              Replaced by HLC-based replay protection (see ticket-64-downlink-cmac).

ALTER TABLE gates DROP COLUMN IF EXISTS last_seq_tx;

COMMENT ON COLUMN gates.app_mac_key IS '16-byte AES-CMAC key (AppMACKey) provisioned per device at flash time';
