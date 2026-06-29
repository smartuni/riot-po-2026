-- Migration: V13__Remove_cmac_columns.sql
-- Description: Remove AES-CMAC columns from gates table.
--              Downlink signing migrated to Ed25519 COSE (ticket-64-downlink-cmac).

ALTER TABLE gates DROP COLUMN IF EXISTS last_seq_tx;
ALTER TABLE gates DROP COLUMN IF EXISTS app_mac_key;
