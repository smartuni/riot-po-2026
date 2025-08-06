


# Ed25519 Key-Setup & TTN-Gerätekonfiguration

Dieses Modul dient zur automatisierten Erstellung von Ed25519-Schlüsseln sowie zur Registrierung von Geräten in einer bestehenden TTN-Anwendung.

---

## Voraussetzungen

* Eine **bestehende LoRaWAN-Application** in [The Things Stack (TTN)](https://www.thethingsnetwork.org/)
* Ein **TTN API Token** mit vollständigen Anwendungsrechten (z. B. über [TTN Console → API Keys](https://console.cloud.thethings.network/))
* Python 3.6 oder neuer
* Abhängigkeiten:

  ```bash
  pip install pynacl requests pyyaml
  ```

---

## Konfigurationsdateien

### 1. `config.yaml`

Enthält die Gerätekonfiguration sowie Basisinformationen zur TTN-Instanz und Anwendung:

```yaml
tti_instance: eu1.cloud.thethings.network  # TTN Instanz (z. B. eu1, nam1, ...)
application_id: my-application             # Name deiner existierenden TTN-Application

sensemates:
  count: 3
  id_prefix: sensemate

sensegates:
  count: 2
  id_prefix: sensegate
```

### 2. `secrets.yaml`

Speichert den TTN-Auth-Token. **Diese Datei sollte nicht versioniert werden.**

```yaml
ttn_auth_token: "NNSXS.XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
```

---

## Nutzung

### Schritt 1: Schlüssel generieren

```bash
python util_scripts/generate_prod_keys.py
```

Dies erzeugt:

* Für jedes Gerät:

    * Einen privaten Ed25519-Schlüssel in `include/secrets/{device_id}_private_key.h`
* Eine zentrale `public_keys.h`, die alle Geräte-IDs (`kid`) und zugehörigen Public Keys enthält

---

### Schritt 2: TTN-Geräte erstellen

```bash
python util_scripts/generate_ttn_devices.py
```

Das Skript:

* Prüft, ob Geräte bereits existieren, und löscht sie gegebenenfalls
* Erstellt neue Geräte mit:

    * zufällig generiertem `DevEUI`
    * `JoinEUI = 0000000000000000` (konfigurierbar)
    * zufälligem `AppKey`
* Legt für jedes Gerät eine entsprechende TTN-Konfigurationsdatei ab:

  ```
  ttn_configs/{device_id}_config.mk
  ```

Die Dateien enthalten `CFLAGS`, die beim Kompilieren verwendet werden können:

```make
CFLAGS += -DCONFIG_LORAMAC_DEV_EUI_DEFAULT=\"A1B2C3D4E5F6A7B8\"
CFLAGS += -DCONFIG_LORAMAC_APP_EUI_DEFAULT=\"0000000000000000\"
CFLAGS += -DCONFIG_LORAMAC_APP_KEY_DEFAULT=\"ABCDEF1234567890...\"
```

---

## Ergebnis

Nach dem Ausführen beider Skripte:

* Sind alle Geräte bei TTN registriert
* Liegen alle Schlüssel (privat + öffentlich) als C-Header vor
* Kann jedes Gerät anhand seiner spezifischen `*_config.mk` direkt in RIOT OS oder anderen C-Projekten eingebunden werden

---

## Hinweis zur Sicherheit

* Private Schlüssel liegen im Klartext in den `.h`-Dateien – diese sollten **nicht in ein öffentliches Repository** gelangen.
* Auch `secrets.yaml` enthält sensible Daten und sollte per `.gitignore` ausgeschlossen sein.

---