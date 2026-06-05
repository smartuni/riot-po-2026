#!/usr/bin/env python3
"""Provision AppMACKey for a LoRaWAN device.

Generates a random 16-byte AES-128 key (AppMACKey) and outputs:
  - C header for firmware provisioning
  - SQL UPDATE statement for backend database
  - JSON format for backend API

Usage:
  python3 provision_app_mac_key.py sensegate-1
  python3 provision_app_mac_key.py sensegate-1 --output-dir ./keys
  python3 provision_app_mac_key.py sensegate-1 --json
"""

import argparse
import json
import os
import secrets
from pathlib import Path


def generate_app_mac_key() -> bytes:
    """Generate a random 16-byte AES-128 key."""
    return secrets.token_bytes(16)


def format_c_header(device_id: str, key: bytes) -> str:
    key_bytes = ", ".join(f"0x{b:02x}" for b in key)
    return f"""#ifndef APP_MAC_KEY_{device_id.upper().replace('-', '_')}_H
#define APP_MAC_KEY_{device_id.upper().replace('-', '_')}_H

#define APP_MAC_KEY_BYTES 16

static const uint8_t app_mac_key[APP_MAC_KEY_BYTES] = {{
  {key_bytes}
}};

#endif /* APP_MAC_KEY_{device_id.upper().replace('-', '_')}_H */
"""


def format_sql_update(device_id: str, key: bytes, gate_id: int) -> str:
    key_hex = key.hex()
    return f"UPDATE gates SET app_mac_key = decode('{key_hex}', 'hex') WHERE id = {gate_id};"


def format_json(device_id: str, key: bytes) -> str:
    return json.dumps({
        "deviceId": device_id,
        "appMacKey": key.hex(),
        "appMacKeyBytes": list(key),
    }, indent=2)


def main():
    parser = argparse.ArgumentParser(description="Provision AppMACKey for a LoRaWAN device")
    parser.add_argument("device_id", help="Device ID (e.g. sensegate-1)")
    parser.add_argument("--gate-id", type=int, default=None,
                        help="Gate ID for SQL UPDATE (default: extract number from device_id)")
    parser.add_argument("--output-dir", "-o", default=None,
                        help="Output directory for C header file")
    parser.add_argument("--json", action="store_true",
                        help="Output as JSON to stdout")
    parser.add_argument("--sql", action="store_true",
                        help="Output SQL UPDATE statement to stdout")

    args = parser.parse_args()
    device_id = args.device_id

    key = generate_app_mac_key()

    gate_id = args.gate_id
    if gate_id is None:
        import re
        match = re.search(r'(\d+)$', device_id)
        if match:
            gate_id = int(match.group(1))
        else:
            gate_id = 1

    if args.output_dir:
        out_dir = Path(args.output_dir)
        out_dir.mkdir(parents=True, exist_ok=True)
        header_path = out_dir / f"app_mac_key_{device_id}.h"
        with open(header_path, "w") as f:
            f.write(format_c_header(device_id, key))
        print(f"Wrote C header: {header_path}")

    if args.json or (not args.output_dir and not args.sql):
        print(format_json(device_id, key))

    if args.sql:
        print()
        print("-- SQL for backend database:")
        print(format_sql_update(device_id, key, gate_id))


if __name__ == "__main__":
    main()
