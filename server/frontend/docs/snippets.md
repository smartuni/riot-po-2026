# Developer Snippets

Patterns drawn from the actual codebase. Follow these when building new features.

---

## Feature Module Structure

```
src/features/{feature}/
  index.js           # Barrel exports
  components/         # React components
  styles/             # CSS (if needed)
```

Barrel file (`index.js`):

```javascript
export { default as ActivityPanel } from './components/ActivityPanel';
```

Import from barrels, not deep paths:

```javascript
import { ActivityPanel } from '../features/activities';
```

---

## REST API Function

```javascript
import { useGetGatesQuery, useAddGateMutation } from '../features/gates/api';

// RTK Query auto-generates React hooks from endpoint definitions
const { data: gates, isLoading } = useGetGatesQuery();
const [addGate] = useAddGateMutation();
```

- Use RTK Query hooks — never create a new Axios instance or `fetch` calls
- Auto-generated hooks: `useGet*Query`, `useAdd*Mutation`, `useUpdate*Mutation`, `useDelete*Mutation`
- Caching, loading states, and error handling are built in

---

## WebSocket Subscription

```javascript
import { Client } from '@stomp/stompjs';

useEffect(() => {
    const stompClient = new Client({
        webSocketFactory: () => new WebSocket('ws://localhost:8080/ws'),
        onConnect: () => {
            stompClient.subscribe('/topic/gates/updates', (message) => {
                const updatedGate = JSON.parse(message.body);
                setGates(prev => {
                    const i = prev.findIndex(g => g.id === updatedGate.id);
                    if (i !== -1) {
                        const next = [...prev];
                        next[i] = updatedGate;
                        return next;
                    }
                    return prev;
                });
            });
        },
    });

    stompClient.activate();

    return () => { stompClient.deactivate(); };
}, []);
```

- Create STOMPJS Client inside `useEffect`, deactivate on cleanup
- Use functional state updates (`prev =>`) for async message handling
- Parse `message.body` with `JSON.parse()` for objects, `parseInt()` for IDs

---

## Auth Guard in Pages

Protected routes use `ProtectedRoute` and `PublicOnlyRoute` wrappers defined in `features/auth/`. No inline auth checks are needed in page components.

```javascript
import { ProtectedRoute, PublicOnlyRoute } from '../features/auth';

// In App.jsx — wrap routes:
<Route path="/dashboard" element={
  <ProtectedRoute roles={['controller', 'viewer']}>
    <DashboardPage />
  </ProtectedRoute>
} />

<Route path="/login" element={
  <PublicOnlyRoute>
    <LoginPage />
  </PublicOnlyRoute>
} />

// Role gating — restrict to controllers only:
<ProtectedRoute roles={['controller']}>
  <AdminPage />
</ProtectedRoute>
```

---

## Form with Validation

```javascript
const [email, setEmail] = useState('');
const [password, setPassword] = useState('');

const isValidEmail = (email) => /\S+@\S+\.\S+/.test(email);
const isValidPassword = (pw) => pw.length >= 6;
const isFormValid = isValidEmail(email) && isValidPassword(password);

const handleSubmit = async (e) => {
    e.preventDefault();
    try {
    const userData = await login(email, password);
    navigate('/dashboard');
    } catch (error) {
        setErrorMessage(error.response?.data?.error || 'An error occurred');
    }
};
```

---

## MUI Dialog

```javascript
function MyDialog({ open, onClose }) {
    const [value, setValue] = useState('');

    const handleSubmit = async () => {
        try {
            await someApiCall(value);
        } catch (err) {
            console.error(err);
        }
        onClose();
        setValue('');
    };

    return (
        <Dialog open={open} onClose={onClose}>
            <DialogTitle>Title</DialogTitle>
            <DialogContent>{/* fields */}</DialogContent>
            <DialogActions>
                <Button onClick={onClose}>Cancel</Button>
                <Button onClick={handleSubmit} variant="contained">Submit</Button>
            </DialogActions>
        </Dialog>
    );
}
```

---

## Polling Fallback

Used in `StatusTablesView` (viewer/guest dashboards) instead of WebSocket:

```javascript
useEffect(() => {
    const load = async () => {
        try {
            const data = await fetchGates();
            setGates(data);
        } catch (error) {
            console.error(error);
        }
    };
    load();
    const id = setInterval(load, 300);
    return () => clearInterval(id);
}, []);
```

---

## Worker ID Access

```javascript
import { useAppSelector } from '../app/store';

const workerId = useAppSelector(state => state.auth.user?.workerId) ?? null;
```

Worker ID comes from Redux `authSlice` — no separate API call needed.

---

## Role-Based Redirect

```javascript
// After login, all authenticated users go to /dashboard
// DashboardPage conditionally renders based on role
navigate('/dashboard');
```

---

## Cookie Utilities

```javascript
import { getCookie } from '../../shared/utils/cookie';

const csrfToken = getCookie('XSRF-TOKEN');  // string | null — used in RTK Query headers

// JWT is HttpOnly — cannot be read via getCookie('jwt')
// Auth state is managed by Redux (authSlice)
```

---

## Bulleted Quick Reference

### STOMP Topics

| Topic | Event | Subscribers |
|---|---|---|
| `/topic/gates/add` | Gate created | StatusTables, StatCards |
| `/topic/gates/delete` | Gate removed | StatusTables, StatCards |
| `/topic/gates/updates` | Gate status changed | StatusTables, StatCards |
| `/topic/gate-activities` | Activity logged | ActivityPanel, StatusTables |
| `/topic/gate-activities/delete` | Activity removed | ActivityPanel, StatusTables |
| `/topic/uplinks` | Uplink from IoT device | StatusTables |

### REST Endpoints

| Method | Endpoint | Feature |
|---|---|---|
| POST | `/auth/login` | auth |
| POST | `/auth/register` | auth |
| GET | `/auth/user-details` | auth |
| PUT | `/auth/user-change` | auth |
| POST | `/auth/logout` | auth |
| GET | `/gates` | gates |
| POST | `/add-gate-ui` | gates |
| PUT | `/update-gate` | gates |
| DELETE | `/gates/{id}` | gates |
| POST | `/{gateId}/{workerId}/request-status-change/` | gates |
| PUT | `/update-priority/{gateId}` | gates |
| GET | `/downlinkcounter/counter` | gates |
| POST | `/downlinkcounter/try-increment` | gates |
| POST | `/downlinkcounter/reset` | gates |
| POST | `api/downlink` | gates |
| GET | `/gate-activities` | activities |
| POST | `/add-activities/` | activities |
| GET | `/notifications` | notifications |
| GET | `/notifications/{workerId}` | notifications |
| POST | `/notifications/{id}/request-read-change` | notifications |
