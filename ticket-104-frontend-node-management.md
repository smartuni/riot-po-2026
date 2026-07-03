## Ticket #104 — Frontend: Node Management Dashboard

Implement a UI in the dashboard to manage nodes and the root key using the existing backend API.

### API Reference

All endpoints are under `VITE_API_BASE_URL/api/nodes` (the Vite proxy strips `/api` and forwards to the backend at `/nodes`).

#### Root Key

**Upload/Upsert root key pair**
```
POST /api/nodes/root-key
Content-Type: application/json

{
  "publicKey": "<base64-encoded 32-byte ed25519 public key>",
  "privateKey": "<base64-encoded 64-byte ed25519 private key seed>"
}
```
Response `200`:
```json
{
  "kid": "server",
  "publicKey": "<base64>",
  "privateKey": "<base64>"
}
```

**Get root key pair**
```
GET /api/nodes/root-key
```
Response `200`:
```json
{
  "kid": "server",
  "publicKey": "<base64>",
  "privateKey": "<base64>"
}
```
Returns `null` body if not set.

#### Nodes

**List all nodes**
```
GET /api/nodes
```
Response `200`:
```json
[
  {
    "id": 1,
    "name": "sensegate-1",
    "publicKey": "<base64-encoded 32-byte ed25519 public key>"
  }
]
```

**Add a node**
```
POST /api/nodes
Content-Type: application/json

{
  "name": "sensegate-1",
  "publicKey": "<base64-encoded 32-byte ed25519 public key>"
}
```
Response `200`:
```json
{
  "id": 2,
  "name": "sensegate-1",
  "publicKey": "<base64>"
}
```
Throws `400` if the public key is not exactly 32 bytes (raw).

**Delete a node**
```
DELETE /api/nodes/{id}
```
Response `200` (no body). Returns `404` if node not found.

### UI Requirements

#### Root Key Section
- Show current root key status (set / not set)
- Upload form: two textareas/inputs for public key and private key (Base64)
- Download/View current keys (read-only display)
- Visual indicator if no root key is configured

#### Nodes Section
- Table/card list of all registered nodes showing:
  - Name
  - Public key (truncated with copy button)
  - Created date
- "Add Node" button → modal/form with:
  - Name field (text input)
  - Public key field (textarea for Base64)
- Delete button per node with confirmation dialog
- Empty state when no nodes exist

### Implementation Notes
- Follow existing patterns in the frontend (`server/frontend/src/`)
- Use the API helper in `src/api.js` (or equivalent) for requests
- Existing pattern: components under `src/features/<feature>/components/`
- Auth is handled automatically (JWT cookie)
- Keys are transmitted as Base64-encoded strings
- The backend validates key sizes (32 bytes public, 64 bytes private raw before Base64 encoding)

### Testing
- Manual: verify all CRUD operations work end-to-end
- All 13 backend tests in `NodeManagementServiceTest` are green
