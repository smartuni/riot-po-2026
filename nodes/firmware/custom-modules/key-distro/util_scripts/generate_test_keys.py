from nacl.signing import SigningKey
from pathlib import Path

# Erzeuge festen Seed für reproduzierbare Test-Keys
FIXED_SEED = b"test_seed_1234567890123456789012"  # genau 32 bytes



def generate_fixed_key():
    return SigningKey(FIXED_SEED)

def main():
    include_dir = Path(__file__).parent.parent / "include"
    include_dir.mkdir(exist_ok=True)

    # Nur ein Test-Device
    sk = generate_fixed_key()
    pk = sk.verify_key

    # test_private_key.h schreiben
    with open(include_dir / "test_private_key.h", "w") as f:
        f.write("#ifndef TEST_PRIVATE_KEY_H\n#define TEST_PRIVATE_KEY_H\n\n")
        f.write('static const uint8_t ownId[] = "test-device";\n\n')
        f.write("static uint8_t ed25519_secret_key[64] = {\n")
        f.write("  " + ", ".join(f"0x{x:02x}" for x in sk.encode()) + "\n};\n\n")
        f.write("static uint8_t ed25519_public_key[32] = {\n")
        f.write("  " + ", ".join(f"0x{x:02x}" for x in pk.encode()) + "\n};\n\n")
        f.write("#endif // TEST_PRIVATE_KEY_H\n")

    # test_public_keys.h schreiben
    with open(include_dir / "test_public_keys.h", "w") as f:
        f.write("#ifndef TEST_PUBLIC_KEYS_H\n#define TEST_PUBLIC_KEYS_H\n\n")
        f.write('#include "ed25519_key_entry.h"\n\n')
        f.write("static const ed25519_public_key_entry_t known_keys[] = {\n")
        f.write("  {\n")
        f.write('    .kid = "test-device",\n')
        f.write(f'    .kid_len = {len("test-device")},\n')
        f.write("    .public_key = {\n")
        f.write("      " + ", ".join(f"0x{x:02x}" for x in pk.encode()) + "\n")
        f.write("    }\n")
        f.write("  },\n")
        f.write("};\n")
        f.write("#define NUM_KNOWN_KEYS (sizeof(known_keys)/sizeof(known_keys[0]))\n\n")
        f.write("#endif // TEST_PUBLIC_KEYS_H\n")

if __name__ == "__main__":
    main()
