# SenseMate Frontend Documentation

A beginner-friendly overview of the SenseMate web dashboard — the browser-based interface for monitoring and controlling IoT-connected flood gates in Hamburg.

---

## Files in this Documentation

| File | Description |
|------|-------------|
| `01-overview.md` | What the Frontend does, tech stack, system context, and key features |
| `02-setup.md` | Prerequisites, installation, development server, testing, and linting |
| `03-architecture.md` | Feature-based folder structure, component tree, and routing |
| `04-state-management.md` | Redux store, RTK Query API, real-time WebSocket middleware |
| `05-authentication.md` | Login/logout flow, JWT cookies, role-based access control |

---

## Suggested Reading Order (for Beginners)

1. **01-overview.md** — Start here. Understand what the system does and how it fits into the bigger picture.
2. **02-setup.md** — Learn how to install and run the project locally.
3. **03-architecture.md** — Explore how the code is organized and how pages are built from smaller pieces.
4. **04-state-management.md** — Understand how data flows through the application, from the backend to the screen.
5. **05-authentication.md** — Learn how users log in, how sessions are maintained, and how access is controlled.

---

## Glossary

| Term | Plain-English Explanation |
|------|--------------------------|
| **React** | A JavaScript library for building user interfaces using reusable pieces called *components*. Think of components like LEGO bricks — you combine them to build a page. |
| **Vite** | A build tool that transforms your source code into optimized files the browser can run. It also provides a lightning-fast development server with instant page reloads when you save changes (called Hot Module Replacement, or HMR). |
| **Redux / Redux Toolkit** | A *state management* library. "State" means data that the application remembers (who is logged in, list of gates, etc.). Redux acts like a central database inside the browser — any component can read from it or write to it. Redux Toolkit is the modern, simpler way to write Redux code. |
| **RTK Query** | A data-fetching add-on for Redux Toolkit. It automatically generates code for calling REST APIs, handles loading spinners and error states, and caches results so the app doesn't make duplicate network requests. |
| **STOMP** | Simple Text Oriented Messaging Protocol — a way for the browser to *subscribe* to real-time messages from the server. Think of it like subscribing to a WhatsApp group: whenever someone posts, you immediately see it. |
| **WebSocket** | A technology for keeping a persistent two-way connection open between the browser and the server. Unlike regular HTTP (request-then-response), WebSocket lets the server *push* data to the browser at any time without being asked. STOMP messages travel over WebSocket connections. |
| **Leaflet** | An open-source JavaScript library for interactive maps. The Frontend uses it to display flood gates as markers on a map of Hamburg. |
| **MUI (Material UI)** | A UI component library that provides pre-built, good-looking buttons, dialogs, tables, and forms. It follows Google's Material Design guidelines. |
| **SPA (Single Page Application)** | A web app that loads once and then dynamically updates the page without full page reloads. All of SenseMate's pages live in a single HTML file — React handles switching between "pages" by swapping components. |
| **JSX** | A syntax extension for JavaScript that looks like HTML. You write `<h1>Hello</h1>` inside JavaScript files, and tools (Vite) convert it into real DOM elements. |
| **JWT (JSON Web Token)** | A secure, signed piece of data that proves a user is logged in. The Frontend never sees the token directly — it's stored in a browser cookie the backend sets. |
| **HttpOnly Cookie** | A browser cookie that JavaScript code *cannot* read. This is a security feature: even if an attacker injects malicious code, they cannot steal the login token. |
| **CSRF (Cross-Site Request Forgery)** | An attack where a malicious website tricks your browser into making unwanted requests. The Frontend includes a CSRF token header on every request, and the backend validates it. |
| **LoRaWAN / TTN** | The long-range radio network and cloud infrastructure that connects the physical flood gate sensors (SenseGate devices) to the internet. The Frontend talks to the backend, which talks to TTN via MQTT. |
| **HMR (Hot Module Replacement)** | Vite's ability to instantly update a running app when you save a file, without losing the current application state. |
| **Playwright** | A tool for automated end-to-end testing. It opens a real browser, clicks buttons, fills forms, and checks that things appear correctly — just like a real user would. |
| **Vitest** | A test runner that works seamlessly with Vite. It runs unit tests (testing individual functions and components in isolation). |
| **ESLint** | A tool that checks your code for errors and enforces consistent style. It's like a spell-checker for code. |
| **SenseGate** | The physical IoT device mounted on a flood gate. It senses whether the gate is open or closed and reports status via LoRaWAN. |
| **SenseMate** | The overall project name. Refers to both the handheld device (hardware) and this web dashboard (software). |
