# Developer Snippets

Patterns drawn from the actual codebase. Follow these when building new features.

---

## Feature Module Structure

```
src/features/{feature}/
  index.js           # Barrel exports
  components/         # React components
  api/                # REST functions (if needed)
  styles/             # CSS (if needed)
```

Barrel file (`index.js`):

```javascript
export { default as RecentActivity } from './components/RecentActivity';
export { fetchActivities, addActivities } from './api/activityApi';
```

Import from barrels, not deep paths:

```javascript
import { RecentActivity } from '../features/activities';
```

---

## REST API Function

```javascript
import { apiClient } from '../../../shared';

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

- Use the shared `apiClient` — never create a new Axios instance
- Named exports, verb-first: `fetch*`, `add*`, `update*`, `delete*`
- Always re-throw after logging

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

Protected pages validate auth inline by reading the JWT cookie and calling the backend:

```javascript
import { apiClient, getCookie } from '../shared';

const [popupOpen, setPopupOpen] = useState(false);
const navigate = useNavigate();

// Set auth header from cookie
var jwt = getCookie("jwt");
if (jwt != null) {
    apiClient.defaults.headers.common['Authorization'] = `Bearer ${jwt}`;
}

// Validate with backend
const loadDetails = async () => {
    try {
        const response = await apiClient.get('/auth/user-details');
        if (response.status !== 200) {
            throw new Error('Request failed');
        }
    } catch (e) {
        setPopupOpen(true);
    }
};

useEffect(() => { loadDetails(); }, []);

return (
    <div>
        {/* ... page content ... */}
        <AlertDialogIllegal open={popupOpen} onClose={() => navigate('/')} />
    </div>
);
```

Guest pages skip the guard and use `HeaderBarGuest` instead of `HeaderBar`.

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
        navigate(userData.role === 'controller' ? '/dashboard' : '/dashboard-view');
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
import { loadWorkerId } from '../features/auth';

const [workerId, setWorkerId] = useState(null);

useEffect(() => {
    loadWorkerId().then(id => setWorkerId(id)).catch(e => console.error(e));
}, []);
```

Always use `?? null` as fallback — workerId may be undefined during loading.

---

## Role-Based Redirect

```javascript
if (userData.role === 'controller') {
    navigate('/dashboard');
} else {
    navigate('/dashboard-view');
}
```

---

## Cookie Utilities

```javascript
import { getCookie, setCookie, eraseCookie } from '../../shared';

const jwt = getCookie('jwt');       // string | null
setCookie('jwt', token);             // 7-day default
setCookie('jwt', token, 14);         // custom days
eraseCookie('jwt');
```

---

## Bulleted Quick Reference

### STOMP Topics

| Topic | Event | Subscribers |
|---|---|---|
| `/topic/gates/add` | Gate created | StatusTables, InfoBoxes |
| `/topic/gates/delete` | Gate removed | StatusTables, InfoBoxes |
| `/topic/gates/updates` | Gate status changed | StatusTables, InfoBoxes |
| `/topic/gate-activities` | Activity logged | RecentActivity, StatusTables |
| `/topic/gate-activities/delete` | Activity removed | RecentActivity, StatusTables |
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
