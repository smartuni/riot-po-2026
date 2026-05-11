# Ed25519 Key-Setup & TTN-Device-configuration
This module automatically generates Ed25519-Keys as well as registers devices inside an existing TTN-Application.
  
> [!CAUTION]
> ### Note regarding security
> Private Keys can be accessed inside `.h`-files – these **should not be accessible from a public repository**.
> `secrets.yaml` also contains sensitive data and should be excluded via `.gitignore`.
# How to: Setting up the keys
## A. Requirements

* An **existing LoRaWAN-Application** in [The Things Stack (TTN)](https://www.thethingsnetwork.org/)
* A **TTN API Token** with full application access (via [TTN Console → API Keys](https://eu1.cloud.thethings.network/console/applications/hawriotfloodgates26/))
* Python 3.6 or later
* Dependencies:
```
pip install pynacl requests pyyaml
```
## B. Makefiles to automate the keys generation
> [!NOTE]
> Only steps 1 and 2 are neccessary if you want to flash the board. Do them only once after you clone the repository. 
### 1. TTN API Key
1. Head to the [TTN Portal](https://eu1.cloud.thethings.network/console/applications/hawriotfloodgates26/) and sign in
2. Press **API Keys**
3. **Add API Key** and copy it
4. Run this command inside **senseGate** or **senseMate** folder and paste in the API key
```
make init-keys
```
This automates the step **C1**
### 2. Generate private & public keys
```
make generate-keys
```
This automates the step **D1** and **D2**
### 3. Deleting generated keys (only for debug reasons)
```
make delete-keys
```

---
# Documentation for manual configuration 
## C. Configuration files

### 1. `config.yaml`
Contains the device configuration as well as basic information regarding the TTN instance and usage:
```yaml
tti_instance: eu1.cloud.thethings.network  # TTN instance (e.g. eu1, nam1, ...)
application_id: my-application             # Name of your existing TTN application
sensemates:
  count: 3
  id_prefix: sensemate  
sensegates:
  count: 2
  id_prefix: sensegate
```
> [!NOTE]
> Replace **my-application** with the ID of your application in TTN Web Portal. For now this has been modified to **hawriotfloodgates26**

### 2. `secrets.yaml`

This file should save the TTN-Auth-Token. 
1. Head to the [TTN Portal](https://eu1.cloud.thethings.network/console/applications/hawriotfloodgates26/) and sign in
2. Press **API Keys**
3. **Add API Key** and copy it
4. Create a file **secrets.yaml** on  
```
nodes/firmware/custom-modules/key-distro/util_scripts/
```
5. and put this in:
```yaml
ttn_auth_token: "NNSXS.XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
```
and replace it with your **API Key**
  

## D. Usage

### 1. Generate Keys
```
cd nodes/firmware/custom-modules/key-distro/util_scripts
python generate_prod_keys.py
```
This generates:
* For every device:
    * A private Ed25519-Key in `include/secrets/{device_id}_private_key.h`
* A central `public_keys.h`, which contains all device-IDs (`kid`) and all associated Public Keys

### 2. Generate TTN-devices

```
cd nodes/firmware/custom-modules/key-distro/util_scripts
python generate_ttn_devices.py
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

## E. Result
After executing both scripts:
* All devices are registered with TTN
* All keys (private & public) can be accessed inside a c-header-file
* Each device can be integrated directly into RIOT OS or another C-Project via its specific `*_config.mk`

