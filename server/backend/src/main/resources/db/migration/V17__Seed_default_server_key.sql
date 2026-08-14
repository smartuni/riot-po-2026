-- Migration: V17__Seed_default_server_key.sql
-- Description: Insert a default Ed25519 server signing key for COSE downlink signing.
-- The server_key table is a singleton (CHECK id=1), and DownlinkService requires
-- a row to exist for signing gate commands.

INSERT INTO server_key (id, kid, private_key, public_key, created_at, updated_at)
VALUES (
    1,
    'ea7d996d0d3cf073',
    '\x2519d176f03abc3196668d3f4a3d2667e8b3f5f3bdb0330e443b9bb53d929eaa',
    '\xea7d996d0d3cf073ef6ece5e50d4d9b04f1150385fd76f2122cf38e8a1abd75a',
    CURRENT_TIMESTAMP,
    CURRENT_TIMESTAMP
);
