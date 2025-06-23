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
export const requestGateStatusChange = async (gateId, status) => {
    try {
        const response = await api.post(
            `/${gateId}/request-status-change`,
            { requestedStatus: status }
        );
        return response.data;
    } catch (error) {
        console.error('Error requesting gate status change:', error);
        throw error;
    }
};

export default api;