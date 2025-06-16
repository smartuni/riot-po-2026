# Rescue‑Mate

## Requirements (Backend)
- Java17
- Maven
- SpringBoot
- Mosquitto (MQTT broker)

---

## Installing Mosquitto

Get the MQTT broker from the official Eclipse Mosquitto download page:

**https://mosquitto.org/download/**

---

## Maven & Spring Boot Setup

Include the Paho MQTT client and Spring Integration MQTT in your `pom.xml`:

```xml
<dependency>
  <groupId>org.eclipse.paho</groupId>
  <artifactId>org.eclipse.paho.client.mqttv3</artifactId>
  <version>1.2.5</version>
</dependency>
<dependency>
  <groupId>org.springframework.integration</groupId>
  <artifactId>spring-integration-mqtt</artifactId>
  <version>6.5.0</version>
</dependency>
```

## What MQTT Does & What to Keep in Mind

### How Rescue‑Mate Webteam Uses MQTT

**MQTT** is a lightweight publish‑subscribe messaging protocol ideal for IoT, where devices (clients) communicate exclusively through a central broker rather than directly.

- **Publishers** (e.g. devices) publish data to structured topics.
- **Mosquitto broker** receives and routes these messages to all subscribers that match the topic filter.
- **Subscribers** (the backend) subscribe to relevant topics and process incoming payloads accordingly.

### Key Considerations with TTN

1. **TTN only allows QoS 0**  
   The Things Stack supports MQTT 3.1.1 and QoS 0 only. Any higher QoS set by the client will be downgraded when passing through TTN.
2. **Use a local Mosquitto bridge**  
   TTN → Mosquitto (bridge, QoS 0) → Spring Boot (subscribe with QoS 1 or 2).
3. **Topic strategy**  
   Subscribe selectively, e.g. `v3/{app}@{tenant}/devices/+/up`, to avoid unnecessary traffic.
4. **Security**  
   Authenticate with TTN using API key; use TLS on both TTN and Mosquitto endpoints (port 8883).

For more Information follow the link:
**https://www.thethingsindustries.com/docs/integrations/other-integrations/mqtt/**

## Recent Updates

- **2025‑06‑15**:
    - Included a Downlink Service that needs to be tested with the Node team.

