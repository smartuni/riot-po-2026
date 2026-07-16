# 02 — Setup & Deployment

## Prerequisites

### 1. Clone the Repository with Submodules

The RIOT OS source code lives as a git submodule. You must clone with `--recurse-submodules`:

```bash
git clone --recurse-submodules <repo-url>
```

If you already cloned without submodules, run:

```bash
./submodules_linux.sh
```

### 2. Toolchain

You need the RIOT OS build tools. The recommended approach is to use the Docker image `riot/riotbuild:2025.07`. If you prefer a native setup, install the ARM GCC toolchain (`gcc-arm-none-eabi`) and all RIOT dependencies as described in the [RIOT OS documentation](https://doc.riot-os.org/getting-started.html).

### 3. Python Dependencies (for identity-manager)

The identity provisioning script uses [uv](https://docs.astral.sh/uv/) for Python dependency management. Install `uv` and run the identity-manager from the `nodes/firmware/identity-manager/` directory.

## Step 1: Generate Signature Keys

**This is mandatory — flashing will fail without these keys.**

The `key-distro` custom module expects cryptographic key material at:
`nodes/firmware/custom-modules/key-distro/`

This directory is **not** part of the repository (it's `.gitignore`d). You must create it and populate it with keys. The directory structure is:

```
nodes/firmware/custom-modules/key-distro/
├── include/
│   └── secrets/
│       └── keys.h       # Cryptographic key definitions
└── ttn_configs/
    └── *.mk             # LoRaWAN/The Things Network configuration
```

Refer to the CI pipeline script (`ci/generate_compile_tests_pipeline.sh:38-40`) for an example of how test keys are generated:

```bash
mkdir -p nodes/firmware/custom-modules/key-distro/include/secrets
cp test/keys/*.h nodes/firmware/custom-modules/key-distro/include/secrets/
cp test/keys/*.mk nodes/firmware/custom-modules/key-distro/ttn_configs/
```

For production deployments, generate real Ed25519 key pairs and configure TTN credentials for your application.

## Step 2: Build and Flash

Navigate to the SenseMate application directory:

```bash
cd nodes/firmware/applications/senseMate/
```

### Test Mode

Flashes a test firmware with hardcoded test data. No device ID is needed:

```bash
make all flash RIOT_CONFIG_USE_TEST=1
```

### Production Mode

You must provide a `DEVICE_ID` (a number that uniquely identifies this specific SenseMate device in the field):

```bash
make all flash RIOT_CONFIG_DEVICE_ID=<DEVICE_ID>
```

The device type is **hardcoded** to `DEVICE_TYPE_SENSEMATE` in the Makefile and cannot be overridden — the `DEVICE_ID` only controls which specific Mate this is (e.g., Mate-1, Mate-2, etc.).

### Default Board

The default target board is `seeedstudio-xiao-nrf52840-sense` (v2). The legacy v1 board (`adafruit-feather-nrf52840-sense`) is still supported through conditional compilation.

### Build Options

| Variable | Default | Purpose |
|----------|---------|---------|
| `BOARD` | `seeedstudio-xiao-nrf52840-sense` | Target hardware board |
| `USE_FLASHDB_ON_VFS` | `0` | `1` = use a filesystem (LittleFS) for FlashDB; `0` = use MTD directly |
| `DEVELHELP` | `1` | Enable RIOT safety checks (disable in production) |

## Step 3: Provision an Identity

After flashing, the device has no identity. It cannot sign messages or join the LoRaWAN network until provisioned.

Use the **identity-manager** script located at `nodes/firmware/identity-manager/`:

### Quickstart (from `nodes/firmware/identity-manager/README.md:22-41`)

1. **Create a root identity** (only needed once, shared by all devices):
   ```bash
   ./identity-manager.py root create
   ```

2. **Copy the config template** and fill in your TTN credentials:
   ```bash
   cp config.example.yaml config.yaml
   # Edit config.yaml with your TTN applicationID and authToken
   ```

3. **Generate a node identity** for this SenseMate:
   ```bash
   ./identity-manager.py node create senseMate
   ```
   This creates a file like `identities/nodes/senseMate_001.node_id_info.json` containing:
   - The node's private key (Ed25519)
   - The node's signed public identity (COSE Sign1)
   - LoRaWAN credentials (JoinEUI, DevEUI, AppKey)
   - A provisioning payload (base64-encoded)

4. **Provision the device** (connect via USB, then run):
   ```bash
   ./identity-manager.py node provision senseMate ID
   ```
   Replace `ID` with the device ID generated in step 3.

### What Happens During Provisioning

The provisioning payload (from `identity_store.h:32-37`) contains:
- `root_identity` — the public key of the trust anchor
- `private_identity` — this node's private key
- `own_signed_identity` — this node's public identity, signed by the root key
- `loramac_keys` — TTN join credentials

After provisioning, these are stored in the device's persistent flash storage under `VFS_DEFAULT_NVM(0) "/identities/"`.

## Common Troubleshooting

| Problem | Likely Cause | Solution |
|---------|-------------|----------|
| `make flash` fails with linker errors | Missing key-distro keys | Generate keys as described in Step 1 |
| `make flash` fails with board error | Wrong board selected | Check `BOARD` variable |
| Device doesn't join LoRaWAN | Not provisioned or wrong TTN credentials | Re-run identity provisioning with correct `config.yaml` |
| Display stays blank | I2C connection issue or wrong display address | Verify I2C address is `0x3c` and display is properly connected |
| Flashing starts but no output visible | Start sleep too short | The Makefile sets `PREFLASH_DELAY=8`; ensure the device is connected before the delay expires |
| `periph_pwm` feature missing | Board doesn't support PWM | The Makefile requires `periph_pwm` (`nodes/firmware/applications/senseMate/Makefile:123`) |
