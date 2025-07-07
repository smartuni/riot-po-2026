from nacl.signing import SigningKey
from pathlib import Path
import json

NUM_DEVICES = 3

def main():
    base_dir = Path(__file__).parent.parent
    secrets_dir = base_dir / "include" / "secrets"
    include_dir = base_dir / "include"

    secrets_dir.mkdir(parents=True, exist_ok=True)

    key_list = []

    for i in range(NUM_DEVICES):
        kid = f"device_{i+1:02d}"
        sk = SigningKey.generate()
        pk = sk.verify_key

        key_list.append({
            "kid": kid,
            "public_key": pk.encode(),
        })

        # private_key pro device schreiben
        with open(secrets_dir / f"{kid}_private_key.h", "w") as f:
            f.write("#ifndef PRIVATE_KEY_H\n#define PRIVATE_KEY_H\n\n")
            f.write(f'static const uint8_t ownId[] = "{kid}";\n\n')
            f.write("static uint8_t ed25519_secret_key[64] = {\n")
            f.write("  " + ", ".join(f"0x{x:02x}" for x in sk.encode()) + "\n};\n\n")
            f.write("static uint8_t ed25519_public_key[32] = {\n")
            f.write("  " + ", ".join(f"0x{x:02x}" for x in pk.encode()) + "\n};\n\n")
            f.write("#endif // PRIVATE_KEY_H\n")

    # public_keys_prod.h generieren
    with open(secrets_dir / "public_keys.h", "w") as f:
        f.write("#ifndef PUBLIC_KEYS_PROD_H\n#define PUBLIC_KEYS_PROD_H\n\n")
        f.write('#include "ed25519_key_entry.h"\n\n')
        f.write("static const ed25519_public_key_entry_t known_keys[] = {\n")
        for entry in key_list:
            kid = entry["kid"]
            pub = entry["public_key"]
            f.write("  {\n")
            f.write(f'    .kid = "{kid}",\n')
            f.write(f"    .kid_len = {len(kid)},\n")
            f.write("    .public_key = {\n")
            f.write("      " + ", ".join(f"0x{x:02x}" for x in pub) + "\n")
            f.write("    }\n")
            f.write("  },\n")
        f.write("};\n")
        f.write("#define NUM_KNOWN_KEYS (sizeof(known_keys)/sizeof(known_keys[0]))\n\n")
        f.write("#endif // PUBLIC_KEYS_PROD_H\n")

if __name__ == "__main__":
    main()
