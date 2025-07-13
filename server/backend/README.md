# Server Team Back-End
This is the readme for the back-end of the Server side team. It will describe the structure of the directory and where to find important files and functions.

## Intro
The back-end of this project serves to provide communication between front-end and nodes. It also handles storing information, such as gates, users, notifications, etc into in-memory databases. It converts data from front-end and sends it to the nodes and vice-versa. 

## Directory Structure
The structure of this back-end directory contains a number of sub-folders with distinct purposes.

Within the /backend directory there is a few important files, namely pom.xml that holds all of the dependeces and the (/src) source directory that contains all the working implementation. 

The /src directory is where almost all of the files are located. It has a number of important files and subdirectories within it, such as the resources folder, which contains information about configuring the application, the databases, and connecting it to the mqtt broker.  

Config subdirectory is responsible for templates for the rest of application to work off of such as well as handling the setup of parts of the application.

Controller and service subdirectories are very closely related. They are responsible for communication between front-end and back-end. Files in the service subdirectory handle initializing, updating, and removing information from the applications' database and controller files provide interfaces for front-end to communicate with service files. 

Entity and model subdirectories are also closely related and they are responsible for providing a format for communication between front-end and back-end. Entities are what back-end works with and modes are what front-end works with. Entities also provide methods for converting entities to models. 

Enums subdirectory is reponsible for providing user defined data types to ensure standards are met across the entire application. 

Exeptions subdirectory enables us to handle errors. By defining these errors, it allows us tell the application what to do, should one of the errors occur.  

Registry subdirectory tracks gatemates and sensemates on the network. 

Repository subdirectory initializes repositories which store information used when the application is running.

README completed by Cosmin Musteata and Simon Korondi.

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

## 

