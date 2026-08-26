# Health-Status Monitoring System – Backend-Dokumentation

Diese Dokumentation beschreibt die Architektur, das Datenmodell sowie den Verarbeitungsfluss des Health-Status-Systems im Backend der Anwendung (`com.riot.matesense`).

---

## 1. Überblick & Systemarchitektur

Das Health-Status-System ist für die Erfassung, Verarbeitung, flüchtige Speicherung und Verteilung von Telemetrie- und Gesundheitsdaten der IoT-Knoten (`SenseGate` / `SenseMate`) zuständig.

Zu den erfassten Parametern gehören:
* **Batteriestatus** (z. B. Laden, Entladen, Niedrig)
* **Batteriespannung** (in Millivolt, mV)
* **Erschütterungs- & Schockereignisse** (`ShockStatus`)

### Architekturkonzept (Ephemeral In-Memory Pattern)
Da Telemetriedaten in hoher Frequenz eingehen und primär der aktuelle Systemzustand interessiert, verfolgt das Backend ein hybrid-ephemeres Konzept:
1. **Echtzeit-Push (MQTT / WebSocket):** Eingehende Nachrichten werden dekodiert und sofort via WebSocket an verbundene Clients gestreamt.
2. **In-Memory Caching:** Der `HealthStatusService` hält den jeweils aktuellsten Zustand pro Knoten im Arbeitsspeicher. Dadurch kann das Frontend beim Initialisieren (z. B. Page Load) den aktuellen Status per HTTP-GET abrufen, ohne auf das nächste MQTT-Event warten zu müssen.

```text 
┌─────────────────┐      MQTT / CBOR      ┌───────────────────┐
│ SenseGate Node  │ ────────────────────► │   JsonFormatter   │
└─────────────────┘                       └─────────┬─────────┘
                                                    │ Standard JSON
                                                    ▼
┌─────────────────┐   WebSocket Broadcast ┌───────────────────┐
│ Client Frontend │ ◄──────────────────── │ MqttMessageHandler│
└─────────────────┘  (/topic/health)      └─────────┬─────────┘
        ▲                                           │ Internal Update
        │ HTTP GET                                  ▼
┌───────┴─────────┐                       ┌───────────────────┐
│ HealthController│ ◄──────────────────── │HealthStatusService│
└─────────────────┘    (In-Memory Map)    └───────────────────┘
````

---

## 2. Datenmodelle & Enums

### `HealthStatusDTO`
Data Transfer Object zur Repräsentation des aktuellen Gesundheitszustands eines `SenseGate`.

| Feld | Typ | Beschreibung |
| :--- | :--- | :--- |
| `version` | `int` | Protokollversion der Nachricht. |
| `senseGateId` | `int` | Eindeutige Identifikationsnummer des `SenseGate`. |
| `shockStatus` | `ShockStatus` | Status bezüglich Erschütterungen / Schock-Events. |
| `batteryStatus` | `BatteryStatus` | Aktueller Lade- bzw. Betriebszustand der Batterie. |
| `voltageMv` | `int` | Gemessene Batteriespannung in Millivolt (mV). |

---

### Enums

#### `MsgType`
Klassifiziert den eingehenden MQTT-Nachrichtentyp. Für Health-Monitoring wird der Code `5` verwendet:
* `HEALTH_MONITORING(5)`

#### `BatteryStatus`
Repräsentiert den Batteriezustand basierend auf den empfangenen Bitcodes:

| Enum-Wert | Hex-Code | Beschreibung |
| :--- | :--- | :--- |
| `CHARGING` | `0x00` | Batterie wird aktuell geladen. |
| `DISCHARGING` | `0x01` | Batterie wird entladen (Normalbetrieb). |
| `LOW_BATTERY` | `0x02` | Kritisch niedriger Akkustand. |
| `UNKNOWN` | `-1` | Fallback bei unbekannten/unvollständigen Daten. |

---

## 3. Datenverarbeitungs-Pipeline

### Schritt 1: Dekodierung (`JsonFormatter`)
Eingehende Binär- bzw. CBOR-Datenpakete werden in der Methode `toJsonFormat` analysiert.

Wenn ein Paket den `messageType == 5` aufweist:
1. Auslesen von `version`, `senseGateId`, `eventHeader` und `eventBody`.
2. Auswertung des `eventHeader`:
    * **`0x00`, `0x01`, `0x02`:** Setzt den entsprechenden `BatteryStatus` und interpretiert `eventBody` als Spannungswert in Millivolt (`voltageMv`).
    * **`0x03`:** Mapping von `eventBody` auf den passenden `ShockStatus`.
    * **`0x04`:** Event für freien Fall (`FREE_FALL`) – vorbereitet für zukünftige Implementierungen.
3. Konstruktion des `HealthStatusDTO` und Generierung eines normierten JSON-Strings.

---

### Schritt 2: Verteilung & In-Memory Update (`MqttMessageHandler`)
In der Methode `msgHandlerUplinks`:
1. **Sicherheitsprüfung:** Es werden nur Nachrichten von Geräten verarbeitet, deren Name mit `sensegate-` oder `sensemate-` beginnt.
2. **Verarbeitung von `MsgType.HEALTH_MONITORING`:**
    * **WebSocket-Broadcast:** Sendet die Nachricht unverzüglich an das WebSocket-Topic `/topic/health`.
    * **In-Memory Speicherung:** Übergibt die ausgelesenen Daten an den `HealthStatusService`.

---

### Schritt 3: Zustandszusammenführung (`HealthStatusService`)
Der `HealthStatusService` speichert die Daten in einer thread-sicheren `ConcurrentHashMap<Integer, HealthStatusDTO>`.

#### Merging-Strategie (`updateHealth`)
Da IoT-Geräte häufig partielle Events senden (z. B. nur eine Schock-Meldung ohne neue Spannungswerte), verhindert eine Merge-Logik das Überschreiben gültiger Zustände mit `UNKNOWN` oder `0`:

```java
// Beispiel: Erhalt von bestehenden Spannungswerten bei reinen Schock-Events
int mergedVoltage = (existing != null && voltageMv == 0 && existing.getVoltageMv() != 0)
        ? existing.getVoltageMv() 
        : voltageMv;
````
## 4. REST-API Schnittstelle
Über den HealthController steht ein REST-Endpunkt zur Verfügung, um den aggregierten Gesamtzustand aller Knoten abzurufen.

Endpunkt: Alle Health-Status-Daten abrufen
* URL: /health

* Methode: GET

* Response Content-Type: application/json

* HTTP Status Code: 200 OK

Beispiel-Response

```json
{
  "101": {
    "version": 1,
    "senseGateId": 101,
    "shockStatus": "NO_SHOCK",
    "batteryStatus": "DISCHARGING",
    "voltageMv": 3750
  },
  "102": {
    "version": 1,
    "senseGateId": 102,
    "shockStatus": "SHOCK_DETECTED",
    "batteryStatus": "LOW_BATTERY",
    "voltageMv": 3200
  }
}
````
