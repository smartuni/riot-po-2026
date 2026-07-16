# Server Backend Documentation

This folder contains beginner-friendly documentation for the **MateSense Backend** — the central server of the IoT floodgate monitoring system.

---

## Files Index

| File | Description |
|---|---|
| `01-overview.md` | What the Backend is, its technology stack, and how it fits into the overall system |
| `02-setup.md` | How to install, configure, run, and test the Backend |
| `03-architecture.md` | Package structure, layered design, and how components are wired together |
| `04-api-endpoints.md` | All REST API endpoints grouped by domain (Auth, Gates, Nodes, etc.) |
| `05-database.md` | Database structure, entities, and Flyway migration strategy |
| `06-authentication.md` | JWT-based login, session management, and security configuration |
| `07-communication.md` | MQTT (gate uplinks/downlinks), WebSocket/STOMP (real-time updates), and device registry |

---

## Suggested Reading Order (for beginners)

1. **`01-overview.md`** — Start here. Understand what the Backend does and how it fits into the bigger picture.
2. **`02-setup.md`** — Learn how to get the Backend running on your machine.
3. **`03-architecture.md`** — Understand the code structure and how the layers work together.
4. **`04-api-endpoints.md`** — See what REST endpoints the Backend provides.
5. **`05-database.md`** — Learn how data is stored and managed.
6. **`06-authentication.md`** — Understand how users log in and how the system protects itself.
7. **`07-communication.md`** — See how the Backend communicates with physical devices (MQTT) and the web frontend (WebSocket).

---

## Glossary

| Term | Meaning |
|---|---|
| **Spring Boot** | A Java framework that makes it easy to create web servers. It handles routing, configuration, and wiring components together. |
| **JPA (Jakarta Persistence API)** | A standard way to map Java objects to database tables. You write Java classes, and JPA generates SQL automatically. |
| **Hibernate** | The most popular JPA implementation. Spring Boot uses it behind the scenes. |
| **Flyway** | A database migration tool. It applies versioned SQL scripts to keep the database schema in sync with the code. |
| **MQTT** | A lightweight messaging protocol designed for IoT devices. Uses a publish/subscribe model where devices send messages to a **broker**, and interested clients subscribe to topics. |
| **Eclipse Paho** | The Java MQTT client library used in this project (`server/backend/pom.xml:55-58`). |
| **The Things Network (TTN)** | A global LoRaWAN network. The physical floodgate sensors send data via LoRaWAN to TTN, which then forwards it via MQTT to our Backend. |
| **LoRaWAN** | A low-power, long-range radio communication protocol. Used by the SenseGate and SenseMate devices to communicate with TTN gateways. |
| **CBOR** | Concise Binary Object Representation — a binary data format similar to JSON but more compact. Used to encode payloads between devices and the server. |
| **COSE** | CBOR Object Signing and Encryption — a standard for signing and encrypting CBOR-encoded data. |
| **STOMP** | Streaming Text Oriented Messaging Protocol — a simple text-based protocol that runs on top of WebSockets. Used here to push real-time updates to the web frontend. |
| **WebSocket** | A protocol that creates a persistent, two-way connection between a browser and server. Enables real-time push without polling. |
| **JWT (JSON Web Token)** | A compact, self-contained token format used for authentication. After login, the server gives the browser a JWT cookie, and the browser sends it with every subsequent request. |
| **HMAC-SHA256** | A cryptographic algorithm used to sign JWTs. The server uses a shared secret to create and verify signatures. |
| **BCrypt** | A password hashing algorithm designed to be slow (resistant to brute-force attacks). Used to store user passwords securely. |
| **REST API** | Representational State Transfer — a style of building web APIs where each URL represents a resource and HTTP methods (GET, POST, PUT, DELETE) represent actions. |
| **DTO (Data Transfer Object)** | A simple Java object used to transfer data between layers (e.g., from controller to client). Unlike entities, DTOs are not mapped to database tables. |
| **Entity** | A Java class that represents a database table row. Each instance corresponds to one row in the database. |
| **Repository** | A Spring Data interface that provides methods to read/write entities to the database (e.g., `findById`, `save`, `delete`). |
| **Dependency Injection (DI)** | A design pattern where an object receives its dependencies from outside rather than creating them itself. Spring Boot manages this automatically via `@Autowired`. |
| **PostgreSQL** | A powerful open-source relational database. Used in production and development. |
| **H2** | A lightweight in-memory database. Used in the `e2e` profile so tests can run without needing PostgreSQL installed. |
| **CORS** | Cross-Origin Resource Sharing — a security mechanism that controls which websites can access the API. Configured in `server/backend/src/main/java/com/riot/matesense/config/WebConfig.java:10-22`. |
| **Actuator** | A Spring Boot module that provides built-in endpoints for monitoring and health checks (e.g., `/actuator/health`). |
