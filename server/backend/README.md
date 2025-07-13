Rescue‑Mate is an IoT-powered backend system that connects emergency devices via **The Things Network (TTN)**. It uses **Spring Boot**, **MQTT**, and **Java 17** to receive and process data efficiently from LoRaWAN devices.

---

## Requirements to Start the Backend

- Java 17
- Maven
- Spring Boot
- TTN Account
- (Optional) Local Mosquitto MQTT Broker
- (Optional) Node.js *(only if required for frontend or downlink services)*

---

## Setup Guide

### 1.  Set Up TTN

To connect Rescue‑Mate to TTN, you first need to access the **TTN Console**, where you can manage your applications and devices.

#### How to open the TTN Console:

1. Go to [https://www.thethingsnetwork.org](https://www.thethingsnetwork.org)
2. Click **“Login”** in the top right and sign in with your TTN account
3. Once logged in, click on your **profile icon (top right)**
4. Select **“Console”** from the dropdown
5. In the console, choose your cluster (e.g. **Europe 1 / eu1**)

> You are now inside the **The Things Stack Console**, where you can register applications, devices, and generate API keys.

Continue with the following steps:

2. **Create an Application**
    - On the left sidebar, select **“Applications”**        
    - On the top right, click **“+ Add application”**
    - Give your application a unique ID, e.g. `rescue-mate-app`
    - Click **“Create application”**


3. **Register Your Device**
    - Inside the application, go to the **“End devices”** tab
    - Click **“+ Add end device”**
    - Choose manual setup or a template
    - Fill in the LoRaWAN settings: **DevEUI**, **AppEUI**, **AppKey**
    - Save and ensure the device sends **uplink** messages


4. **Generate an API Key**
    - Inside your application, go to **“API Keys”**
    - Click **“+ Add API key”**
    - Select permission: **“Read application traffic”**
    - Copy and store the API key — this will be used as your **MQTT password**
    -

5. **Configure MQTT in `application.yml`**

After setting up your TTN application and generating an API key, configure your MQTT connection in Spring Boot using the `application.yml` file.  
You can find this file here: `src/main/resources/application.yml`

```yaml
mqtt:
  broker: ssl://eu1.cloud.thethings.network:8883
  clientId: mqtt-client-1234 <-- don't need to be changed
  username: your-app-id@ttn
  applicationId: your-app-id
  password: your-ttn-api-key
  subscribeTopic: v3/your-app-id@ttn/devices/+/up
```

More info: [TTN MQTT Integration Guide](https://www.thethingsindustries.com/docs/integrations/)

---

### 2. Maven & Spring Boot Setup

Add the following dependencies to your `pom.xml`:

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
2. **Topic strategy**  
   Subscribe selectively, e.g. `v3/{app}@{tenant}/devices/+/up`, to avoid unnecessary traffic.
3. **Security**  
   Authenticate with TTN using API key; use TLS on both TTN and Mosquitto endpoints (port 8883).

For more Information follow the link:
**https://www.thethingsindustries.com/docs/integrations/other-integrations/mqtt/**



## Known Issues & Troubleshooting

Even with correct setup and credentials, some problems may still occur. Here are some known issues and how to resolve them:

### 1. MQTT Connection Fails Despite Correct Setup

Sometimes the MQTT connection to TTN fails, even when `application.yml` is correct and the API key is valid. This could be caused by **network interference or restrictions** (e.g. corporate firewalls, university networks).

**Solutions:**
- Try using a different network (e.g. mobile hotspot or private Wi-Fi)
- Make sure port **8883** is not blocked
- Ensure your system clock is accurate (some TLS setups require time sync)

---

### 2. `application.yml` Not Loaded or Ignored

If your configuration seems correct but is not applied, it may be due to incorrect **project structure** or **resource folder setup**.

**Solutions:**
- Make sure `src/main/resources/` is marked as a **Resource Root** in your IDE
- In IntelliJ IDEA: right-click the `resources` folder → *Mark Directory as* → *Resources Root*
- In other IDEs, you may need to configure the classpath manually

---

### 3. Maven Project Not Recognized

If Maven dependencies are not resolved:

**Solutions:**
- Ensure your project is properly imported as a **Maven project**
- In IntelliJ: right-click `pom.xml` → *Add as Maven Project*
- Run `mvn clean install` to verify the build

---

### Recommendation: Use IntelliJ IDEA

We recommend using **IntelliJ IDEA** for this project, as it was used during development and supports:
- Easy marking of `resources` and `sources`
- Maven integration
- YAML highlighting
- Spring Boot auto-completion

> Other IDEs can be used, but may require additional manual configuration.


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