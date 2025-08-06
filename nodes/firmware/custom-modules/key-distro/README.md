


# Ed25519 Key-Setup & TTN-Device-configuration

This module automatically generates Ed25519-Keys as well as registers devices inside an existing TTN-Application.

---

## Requirements

* An **existing LoRaWAN-Application** in [The Things Stack (TTN)](https://www.thethingsnetwork.org/)
* A **TTN API Token** with full application accessn (e.g. via [TTN Console → API Keys](https://console.cloud.thethings.network/))
* Python 3.6 or later
* Dependencies:

  ```bash
  pip install pynacl requests pyyaml
  ```

---

## Configuration files

### 1. `config.yaml`

Contains the device configuration as well as basic information regarding the TTN instance and usage:

```yaml
tti_instance: eu1.cloud.thethings.network  # TTN instance (e.g. eu1, nam1, ...)
application_id: my-application             # Name of your existing TTN application

sensemates:
  count: 3
  id_prefix: sensemate

sensegates:
  count: 2
  id_prefix: sensegate
```

### 2. `secrets.yaml`

Saves TTN-Auth-Token. **Don't add version numbers to this file**

```yaml
ttn_auth_token: "NNSXS.XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
```

---

## Usage

### Step 1: Generate Keys

```bash
python util_scripts/generate_prod_keys.py
```

This generates:

* For every device:

    * A private Ed25519-Key in `include/secrets/{device_id}_private_key.h`
* A central `public_keys.h`, which contains all device-IDs (`kid`) and all associated Public Keys

---

### Step 2: Generate TTN-devices

```bash
python util_scripts/generate_ttn_devices.py
```

Script:

* Checks whether devices already exists and deletes them if necessary
* Generates new devices with:

    * randomly generated `DevEUI`
    * `JoinEUI = 0000000000000000` (configurable)
    * random `AppKey`
* Generates a TTN-configuration file for each device

  ```
  ttn_configs/{device_id}_config.mk
  ```

The files contain `CFLAGS`, which can be used at compilation:

```make
CFLAGS += -DCONFIG_LORAMAC_DEV_EUI_DEFAULT=\"A1B2C3D4E5F6A7B8\"
CFLAGS += -DCONFIG_LORAMAC_APP_EUI_DEFAULT=\"0000000000000000\"
CFLAGS += -DCONFIG_LORAMAC_APP_KEY_DEFAULT=\"ABCDEF1234567890...\"
```

---

## Result

After executing both scripts:

* All devices are registered with TTN
* All keys (private & public) can be accessed inside a c-header-file
* Each device can be integrated directly into RIOT OS or another C-Project via its specific `*_config.mk`

---

## Note regarding security

* Private Keys can be accessed inside `.h`-files – these **should not be accessible from a public repository**.
* `secrets.yaml` also contains sensitive data and should be excluded via `.gitignore`.

---