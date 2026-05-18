# Data Flow Patterns

The application uses three primary data flow patterns. Each serves a different purpose and use case.

## 1. REST API Flow (Synchronous CRUD)

Used for: Login, registration, gate CRUD, downlink commands, user profile updates, notification management.

```mermaid
sequenceDiagram
    participant User
    participant Component
    participant API as API Function
    participant Axios
    participant Backend
    
    User->>Component: Click button
    Component->>API: Call API fn
    API->>Axios: apiClient.get/post/put/delete
    Axios->>Backend: HTTP request
    Backend-->>Axios: Response
    Axios-->>API: response.data
    API-->>Component: Return data
    Component->>Component: setState(data)
    Component-->>User: UI updated
```

### Example: Changing a Gate Status

```javascript
// 1. Dialog component captures user intent
const handleStatusChange = async (gateId, newStatus) => {
  try {
    // 2. Call feature API function
    const result = await requestGateStatusChange(gateId, workerId, newStatus);
    // 3. API function calls Axios
    //    → POST /{gateId}/{workerId}/request-status-change/
    //    with body: { requestedStatus: newStatus }
    // 4. On success, WebSocket will push the update
    //    (no manual state update needed for the changed gate)
  } catch (err) {
    console.error('Status change failed:', err);
  }
};
```

### All REST Endpoints Used

| Method | Endpoint | Purpose | Feature |
|---|---|---|---|
| POST | `/auth/login` | User login | auth |
| POST | `/auth/register` | User registration | auth |
| GET | `/auth/user-details` | Load user profile | auth |
| PUT | `/auth/user-change` | Update user details | auth |
| POST | `/auth/logout` | Logout | auth |
| GET | `/gates` | List all gates | gates |
| POST | `/add-gate-ui` | Create new gate | gates |
| PUT | `/update-gate` | Update gate | gates |
| DELETE | `/gates/{id}` | Delete gate | gates |
| POST | `/{gateId}/{workerId}/request-status-change/` | Request gate status change | gates |
| PUT | `/update-priority/{gateId}` | Update gate priority | gates |
| GET | `/downlinkcounter/counter` | Get downlink counter | gates |
| POST | `/downlinkcounter/try-increment` | Increment downlink counter (fails at 10) | gates |
| POST | `/downlinkcounter/reset` | Reset downlink counter | gates |
| POST | `api/downlink` | Send downlink command to IoT devices | gates |
| GET | `/gate-activities` | List gate activities | activities |
| POST | `/add-activities/` | Add activity log | activities |
| GET | `/notifications` | List all notifications | notifications |
| GET | `/notifications/{workerId}` | List by worker ID | notifications |
| POST | `/notifications/{id}/request-read-change` | Mark as read | notifications |

## 2. WebSocket Push Flow (Real-Time)

Used for: Live gate status updates, new activities, uplink events.

```mermaid
sequenceDiagram
    participant Backend
    participant Broker as STOMP Broker
    participant WS as WebSocket
    participant Component
    participant UI
    
    Backend->>Broker: Gate status changed
    Broker->>WS: Publish to /topic/gates/updates
    WS->>Component: onMessage()
    Component->>Component: setState()
    Component->>UI: Re-render
```

### Which Components Subscribe to What

| Component | Topics | Trigger |
|---|---|---|
| `StatusTables` | gates/add, gates/delete, gates/updates, gate-activities, gate-activities/delete, uplinks | Any gate or activity change |
| `InfoBoxes` | gates/add, gates/delete, gates/updates | Gate count changes |
| `RecentActivity` | gate-activities, gate-activities/delete | New activity logged or removed |

## 3. Polling Flow (Fallback)

Used for: Read-only dashboards (`DashboardViewPage`, `DashboardGuestPage`).

```mermaid
sequenceDiagram
    participant Component
    participant API
    participant Backend
    
    Component->>Component: useEffect() mounts
    loop Every 300ms
        Component->>API: fetchGates()
        API->>Backend: GET /gates
        Backend-->>API: Response
        API-->>Component: gates data
        Component->>Component: setGates()
    end
    Component->>Component: component unmounts
    Component->>Component: clearInterval()
```

## Error Handling Pattern

API errors are caught at the call site and logged to the console. There is no centralized error handling:

```javascript
export const fetchGates = async () => {
  try {
    const response = await apiClient.get('/gates');
    return response.data;
  } catch (error) {
    console.error('Error fetching gates:', error);
    throw error;
  }
};
```

**In components**, errors from API calls are typically handled with:
1. `console.error()` for logging
2. `alert()` or a dialog for user-facing errors
3. `throw` in API functions so callers can decide what to show

## Specialized Flows

### Downlink Command Flow

The downlink command sends a payload to IoT devices with a rate-limiting counter:

```mermaid
sequenceDiagram
    participant User
    participant StatusTables
    participant CounterAPI as Downlink Counter API
    participant DownlinkAPI as Downlink API
    participant Backend

    User->>StatusTables: Click "Send Downlink"
    StatusTables->>CounterAPI: tryIncrementDownlinkCounter()
    CounterAPI->>Backend: POST /downlinkcounter/try-increment
    alt Counter < 10
        Backend-->>CounterAPI: true
        StatusTables->>DownlinkAPI: sendDownlink(payload)
        DownlinkAPI->>Backend: POST api/downlink
        Backend-->>DownlinkAPI: Success
        StatusTables->>CounterAPI: fetchDownlinkCounter()
        CounterAPI->>Backend: GET /downlinkcounter/counter
        Backend-->>StatusTables: Updated count
    else Counter >= 10
        Backend-->>CounterAPI: false / error
        StatusTables->>User: "Downlink limit reached"
    end
```

The payload structure sent to the downlink API:

```javascript
const payload = [
    0,                                          // Command type
    Math.floor(Date.now() / 1000),             // Timestamp (Unix seconds)
    filteredGates
        .filter(g => g.requestedStatus in statusIntMap)
        .map(g => [g.id, statusIntMap[g.requestedStatus], g.priority ?? 0])
];
```

### Notification Flow

```mermaid
sequenceDiagram
    participant User
    participant HeaderBar
    participant NotifAPI as Notification API
    participant Backend

    HeaderBar->>NotifAPI: fetchNotificationByWorkerId(workerId)
    NotifAPI->>Backend: GET /notifications/{workerId}
    Backend-->>HeaderBar: Notification list

    User->>HeaderBar: Click bell icon
    HeaderBar->>HeaderBar: Show NotificationPopup

    User->>HeaderBar: Click notification
    HeaderBar->>NotifAPI: markNotificationAsRead(id)
    NotifAPI->>Backend: POST /notifications/{id}/request-read-change
    HeaderBar->>HeaderBar: Update local notification state (read: true)
    HeaderBar->>HeaderBar: Show notification detail in Dialog
```

### Bulk Status Change Flow

```mermaid
sequenceDiagram
    participant User
    participant StatusTables
    participant API as Gate API
    participant Backend

    User->>StatusTables: Select bulk status + "Apply to Filtered"
    loop For each filtered gate
        StatusTables->>API: requestGateStatusChange(gateId, workerId, status)
        API->>Backend: POST /{gateId}/{workerId}/request-status-change/
    end
    StatusTables->>API: fetchGates()
    API->>Backend: GET /gates
    Backend-->>StatusTables: Updated gate list
    Note over StatusTables: WebSocket will also push updates<br/>but explicit refetch ensures consistency
```

## Data Flow Summary

```mermaid
flowchart TB
    UI["User Interface<br/>(Pages + Feature Components)"]
    
    REST[("REST API<br/>(Axios)")]
    WS[("WebSocket<br/>(STOMP)")]
    
    State["Component State<br/>(useState)"]
    Render["React Re-render<br/>(Virtual DOM → DOM)"]
    
    UI --> REST
    UI --> WS
    REST --> State
    WS --> State
    State --> Render
```
