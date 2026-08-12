# IoT & Firmware CI/CD Monorepo

Dieses Repository enthält ein verteiltes IoT-System bestehend aus **Firmware (RIOT OS)**, einem **Java Backend** und einem **Node.js Frontend**. Es nutzt eine umfassende GitHub Actions CI/CD-Pipeline für automatisierte Builds, Unit-Tests, End-to-End-Tests und **Hardware-in-the-Loop (HIL)** Testing auf echter Microcontroller-Hardware.

---

## Architektur & Projektstruktur

Das Projekt ist als Monorepo strukturiert:

```text
.
├── nodes/
│   └── firmware/
│       └── tests/            # C/C++ Firmware-Tests (RIOT OS)
├── server/
│   ├── backend/              # Java 17 (Maven / Spring Boot)
│   ├── frontend/             # Node.js (Vite / React/Vue + Playwright)
│   └── docker-compose.yml    # Full-Stack Orchestrierung für lokale Devs & Smoke-Tests
└── tools/
    └── board_helper          # Helper-Skript für Flashing/Hardware-Access
````
##  CI/CD Pipeline (GitHub Actions)
Die Pipeline (.github/workflows/ci.yml) sichert die Softwarequalität über vier Hauptbereiche ab:
```text
                  ┌─────────────────────────────────────────┐
                  │          git push / dispatch            │
                  └────────────────────┬────────────────────┘
                                       │
      ┌────────────────────┬───────────┴───────────┬────────────────────┐
      ▼                    ▼                       ▼                    ▼
┌──────────────┐   ┌──────────────┐        ┌──────────────┐   ┌───────────────────┐
│ Firmware HIL │   │ Backend Unit │        │ Backend Integration││ Frontend E2E   │
│  Discovery   │   │    Tests     │        │  (PostgreSQL)│   │  (Playwright)     │
└──────┬───────┘   └──────┬───────┘        └──────┬───────┘   └───────────────────┘
       │                  │                       │
       ▼                  └───────────┬───────────┘
┌──────────────┐                      ▼
│ Hardware Test│             ┌───────────────────┐
│ (Self-Hosted)│             │  Docker Build     │
└──────────────┘             └─────────┬─────────┘
                                       ▼
                             ┌───────────────────┐
                             │ Docker Smoke Test │
                             │  (Docker Compose) │
                             └───────────────────┘
````
## Pipeline Jobs im Detail
1. Firmware Hardware Tests (HIL)

* Discovery: Durchsucht nodes/firmware/tests/ nach Testordnern und prüft in den Makefiles nach definierten Boards (ignoriert native).

* Execution: Läuft auf einem Self-Hosted Runner mit dem Label hil. Compiliert, flasht (make flash) und testet (make test) den Code direkt auf der physikalisch angeschlossenen Hardware.

2. Backend Tests

* Unit Tests: Führt ./mvnw verify mit Java 17 (Temurin) aus.

* Integrationstests: Startet einen PostgreSQL-Service-Container (postgres:16) und testet die Datenbankinteraktionen.

3. Frontend E2E Tests

* Installiert Node.js 20 und Playwright (Chromium).

* Führt End-to-End-Browser-Tests aus und lädt den Playwright-Report als Build-Artefakt hoch.

4. Docker Builds & Smoke Tests

* Baut Container-Images für Backend und Frontend.

* Führt mit docker compose einen Integrationstest durch, prüft die Health-Endpunkte (/actuator/health) und stellt sicher, dass das Frontend erreichbar ist.

## Lokale Entwicklung
Prerequisites
Java 17 & Maven

Node.js 20 & npm

Docker & Docker Compose

GNU Make & Toolchain für RIOT OS (falls Firmware lokal entwickelt wird)

## Backend starten
cd server/backend

./mvnw clean spring-boot:run
## Frontend starten
cd server/frontend

npm install

npm run dev
## Full-Stack lokal mit Docker starten
cd server

docker compose up --build

Das System ist anschließend unter folgenden Adressen erreichbar:

Frontend: http://localhost:3000

Backend API: http://localhost:8080

Backend Healthcheck: http://localhost:8080/actuator/health
## Hardware-in-the-Loop (HIL) Manuell Ausführen
Hardware-Tests werden bei normalen push-Events automatisch übersprungen, da sie nur ausgeführt werden können, wenn der HIL runner läuft.

Um die Hardware-Tests manuell auf GitHub auszulösen:

Gehe im Repository auf den Reiter Actions.

Wähle den Workflow CI aus.

Klicke auf Run workflow.

Aktiviere die Option Run firmware hardware tests on the self-hosted HIL runner.

Wähle optional das Serial Device aus (Standard: /dev/ttyACM0).