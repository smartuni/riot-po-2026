import axios from 'axios';

// Basis-URL deines Backends
const API_BASE_URL = 'http://localhost:8080';


// Erstellt eine Axios-Instanz mit Standard-Headers
const api = axios.create({
    baseURL: API_BASE_URL,
    headers: {
        'Content-Type': 'application/json',
        'Accept': 'application/json'
    },
});

// Beispiel: Alle Gates abrufen
export const fetchGates = async () => {
    try {
        const response = await api.get('/gates');
        return response.data;
    } catch (error) {
        console.error('Error fetching gates:', error);
        throw error;
    }
};

export const fetchNotificationByWorkerId = async (workerId) => {
    try {
        const response = await api.get(`/notifications/${workerId}`);
        return response.data;
    } catch (error) {
        console.error('Error fetching notifications:', error);
        throw error;
    }
};

export const fetchNotification = async () => {
    try {
        const response = await api.get('/notifications');
        return response.data;
    } catch (error) {
        console.error('Error fetching notifications:', error);
        throw error;
    }
};

export const fetchActivities = async () => {
    try {
        const response = await api.get('/gate-activities');
        return response.data;
    } catch (error) {
        console.error('Error fetching gate-activities:', error);
        throw error;
    }
};

export const addActivities = async (newActivities) => {
    try {
        const response = await api.post(`/add-activities/`, newActivities);
        return response.data;
    } catch (error) {
        console.error('Error adding activities:', error);
        throw error;
    }
}

export const loadWorkerId = async () => {
    try {
        const response = await api.get('/auth/user-details');
        if (response.status !== 200) {
            throw new Error('Request failed with status code ' + response.status);
        }
        return response.data.workerId;
    } catch (e) {
        console.error("Fehler beim Laden der User-Details:", e);
    }
};



// Gate-Update mit besserer Fehlerbehandlung
export const updateGate = async (gateId, gate) => {
    try {
        const response = await api.put('/update-gate', gate);
        return response.data;
    } catch (error) {
        console.error('Error updating gate:', error);
        throw error;
    }
};

// Status-Änderung mit Token-Validierung
export const requestGateStatusChange = async (gateId, workerId,status) => {
    try {
        const response = await api.post(
            `/${gateId}/${workerId}/request-status-change/`,
            { requestedStatus: status }
        );
        return response.data;
    } catch (error) {
        console.error('Error requesting gate status change:', error);
        throw error;
    }
};

export const markNotificationAsRead = async (notificationId) => {
    try {
        console.log("Markiere Notification als gelesen:", notificationId); // zum Debuggen
        await api.post(`/notifications/${notificationId}/request-read-change`);
    } catch (error) {
        console.error("Fehler beim Aktualisieren der Benachrichtigung:", error);
        throw error;
    }
};

export const updateGatePriority = async (gateId, newPriority) => {
    return await api.put(`/update-priority/${gateId}`, {
        priority: newPriority
    });
};

export const fetchDownlinkCounter = async () => {
    const response = await api.get("/downlinkcounter/counter");
    return response.data;
};

export const tryIncrementDownlinkCounter = async () => {
    const response = await api.post("/downlinkcounter/try-increment");
    console.log(response);
    return response.data; // true oder false
};





export default api;