26.05.2025
----------
Aufgaben:
<ol>
  <li>TTN - MQTT, API - Translation layer CBOR/JSON in direction from/to backend server: Jannik, Paul</li>
  <li>BLE - Modul und CBOR, Test BLE Empfang mit mehreren Geräten, Beaconing mit mehreren Geräten: Tini, Nico, Stepan, Maxim</li>
  <li>LoRaWAN - Modul mit API zum Senden / Empfangen</li>
  <li>Datenstruktur (CBOR) - Aufbau, Dokumentation: Tini, Nico, Stepan, Maxim</li>
  <li>Allgemeine Kommunikationsschnittstelle für BLE / LoRaWAN: Tini, Nico, Stepan, Maxim</li>
</ol> 

Notizen:
<ol>
  <li>Git Repo mit Exercises: $ git clone --recurse-submodules https://github.com/inetrg/exercises.git</li>
  <li>Pull Request mit Exercises für:
    <ol>
      <li>BLE: $ git fetch origin pull/12/head:new_exercises</li>
      <li>LoRaWAN: $ git fetch origin pull/13/head:new_exercises</li>
    </ol>
  </li>
  <li>CBOR Decoder für The Things Network: https://github.com/lupyuen/cbor-the-things-network</li>
  <li>LoRaWAN - TTN - General connection details:</li>
      <li>TTN->Application->DEVICE_NAME->Other integrations: Connection details, option to create API key</li>
      <li>Tenant ID: ttn</li>
      <li>RIOT OS doesn't need any MQTT client, just use provided LoRaWAN interface to publish messages</li>
      <li>Important: HAW network blocks connection to TTN, use other WIFI hotspot</li>
      <li>Subscribe to all topcis with MQTT Client (Mosquito): mosquitto_sub -h "eu1.cloud.thethings.network" -t "#" -u "<Username from MQTT section in TTN portal>" -P "<API-KEY>" -d => "#" placeholder for all topics, -d flag shows topic messages</li>
      <li>Publish downlink: mosquitto_pub -h eu1.cloud.thethings.network -p <PORT> -t "v3/MQTT-USERNAME/devices/DEVICE-ID/down/push" -u "<MQTT-USERNAME>" -P "<API-KEY>" -m "{\"downlinks\":[{\"f_port\":1,\"frm_payload\":\"vu8=\",\"priority\":\"NORMAL\"}]}" -d => f_port for logic control e.g. port no. is mapped to a specific function, payload must be encoded in base64</li>
      <li>Tool for hexadecimal to base64 convertion: https://v2.cryptii.com/hexadecimal/base64</li>
      <li>Relevant payload must be extracted from topic received from TTN => JSON key: uplink_message</li>
      <li>Further recources (incl. possible topics): https://www.thethingsindustries.com/docs/integrations/other-integrations/mqtt/</li>
</ol> 