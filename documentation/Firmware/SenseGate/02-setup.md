# 02 -- Setup

This guide walks you through setting up the SenseGate firmware on a real device.

## Prerequisites

### 1. Clone the Repository

The repository uses **git submodules** for RIOT OS. Clone with the `--recurse-submodules` flag:

```bash
git clone --recurse-submodules <repo-url>
```

If you already cloned without submodules, run:

```bash
./submodules_linux.sh
```

This pulls the RIOT OS source code into `nodes/firmware/RIOT/`.

### 2. Build Toolchain

The firmware uses the ARM GCC toolchain for compiling code that runs on the nRF52840. Install it as follows:

```bash
# Ubuntu/Debian
sudo apt install gcc-arm-none-eabi gdb-multiarch

# Or use the official Docker image (used in CI):
docker pull riot/riotbuild:2025.07
```

### 3. Python / Identity Manager Dependencies

The identity manager (used later for provisioning) requires Python and **[uv](https://docs.astral.sh/uv/)**. Install uv:

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

## Step 1: Generate Signature Keys

Before any device can be flashed, you must generate **cryptographic signature keys**. These keys are used to sign sensor data so the backend can verify it hasn't been tampered with.

Key generation is handled by the **identity-manager** (see Step 3). The AGENTS.md also references a `nodes/firmware/custom-modules/key-distro/` module for in-tree key distribution -- if present, consult its README as well. Without valid keys, the build will fail.

> **Note:** In most setups, you run `./identity-manager.py root create` to generate keys, then provision them to the device after flashing. If the build still fails with missing keys, check the key-distro module or project documentation for the latest key generation procedure.

## Step 2: Build and Flash

The default board is the **Seeed Studio XIAO nRF52840 Sense** (v2). All commands are run from the SenseGate application directory:

```bash
cd nodes/firmware/applications/senseGate
```

### Testing Mode (no device ID needed)

```bash
make all flash RIOT_CONFIG_USE_TEST=1
```

This builds the firmware in test mode -- all features are enabled but no specific device identity is required. Use this during development.

### Production Mode (with device ID)

```bash
make all flash RIOT_CONFIG_DEVICE_ID=<GATE_ID>
```

Replace `<GATE_ID>` with a numeric ID for this gate (e.g., `RIOT_CONFIG_DEVICE_ID=1`). The device type is hardcoded to `DEVICE_TYPE_GATE` in the Makefile and cannot be changed.

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BOARD` | `seeedstudio-xiao-nrf52840-sense` | Target hardware board |
| `RIOT_CONFIG_USE_TEST` | unset | Set to `1` for test mode |
| `RIOT_CONFIG_DEVICE_ID` | unset | Numeric ID for the gate node |
| `USE_FLASHDB_ON_VFS` | `0` | Set to `1` to use a filesystem-backed database instead of raw flash access |
| `DEVELHELP` | `1` | Enables safety checks (turn off for production) |

Example with custom options:

```bash
make all flash RIOT_CONFIG_DEVICE_ID=3 USE_FLASHDB_ON_VFS=1 DEVELHELP=0
```

Additional Commands
`make all flash` or with terminal output `make all flash term`

 Complete Flashing
 `make all flash -j BUILD_IN_DOCKER=1`

## Step 3: Provision a Node Identity

After flashing, each SenseGate device needs a unique **identity** including cryptographic keys and LoRaWAN network credentials. This is done with the **identity-manager** Python script.

The identity manager lives at `nodes/firmware/identity-manager/`. It handles:

- Creating cryptographic keys unique to each node
- Registering devices on The Things Network (TTN)
- Provisioning the keys and credentials onto the flashed device

### Quickstart

```bash
cd nodes/firmware/identity-manager

# 1. Copy and edit the configuration file
cp config.example.yaml config.yaml
# Edit config.yaml with your TTN credentials

# 2. Create a root identity (one-time, if not already done)
./identity-manager.py root create

# 3. Create a new node identity for this gate
./identity-manager.py node create senseGate

# 4. If the node was previously provisioned, wipe it first
./identity-manager.py node wipe

# 5. Provision the node (replace ID with the ID from step 3)
./identity-manager.py node provision senseGate ID
```

The identity manager stores generated identities in JSON files under `nodes/firmware/identity-manager/identities/`.

### What Gets Provisioned

Each node identity includes (`nodes/firmware/identity-manager/README.md:98-127`):

- **`kid`**: A unique Key ID for this node
- **`privateKey`**: The node's private key (for signing data)
- **`publicKey`**: The node's public key
- **`signedPublicIdentity`**: The public key cryptographically signed by the root key (proves the node is legitimate)
- **`ttn`**: LoRaWAN credentials (`devEUI`, `joinEUI`, `appKey`)
- **`provisioningPayload`**: A base64-encoded package of all the above, sent to the device

Additionally, the **root public key** is provisioned so the device can verify messages from other trusted nodes.

## Troubleshooting

### Build fails with "signature keys not found"

Signature keys must be generated via the key-distro module before building. See Step 1.

### Flash fails / device not detected

1. Make sure the device is connected via USB and powered on.
2. Check that the correct board is selected (`BOARD=seeedstudio-xiao-nrf52840-sense`).
3. For the XIAO board, double-tap the reset button to enter bootloader mode before flashing.

### LoRaWAN join fails after provisioning

1. Verify the TTN credentials in `config.yaml` are correct.
2. Check that the device is registered in the TTN console with correct `DevEUI`, `JoinEUI`, and `AppKey`.
3. Ensure the device is within range of a LoRaWAN gateway.

### Blue LED never turns on

The blue LED indicates update processing. If it stays off indefinitely:
- The main event loop may have crashed. Re-flash the device.
- Check the serial output for error messages (connect via USB and use a serial monitor at 115200 baud).
