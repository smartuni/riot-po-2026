import apiClient from '../../../shared/api/apiClient';

// Beispiel: Alle Gates abrufen
export const fetchGates = async () => {
    try {
        const response = await apiClient.get('/gates');
        return response.data;
    } catch (error) {
        console.error('Error fetching gates:', error);
        throw error;
    }
};

// Gate-Update mit besserer Fehlerbehandlung
export const updateGate = async (gateId, gate) => {
    try {
        const response = await apiClient.put('/update-gate', gate);
        return response.data;
    } catch (error) {
        console.error('Error updating gate:', error);
        throw error;
    }
};

// Status-Änderung mit Token-Validierung
export const requestGateStatusChange = async (gateId, workerId, status) => {
    try {
        const response = await apiClient.post(
            `/${gateId}/${workerId}/request-status-change/`,
            { requestedStatus: status }
        );
        return response.data;
    } catch (error) {
        console.error('Error requesting gate status change:', error);
        throw error;
    }
};

export const updateGatePriority = async (gateId, newPriority) => {
    return await apiClient.put(`/update-priority/${gateId}`, {
        priority: newPriority
    });
};

export const fetchDownlinkCounter = async () => {
    const response = await apiClient.get("/downlinkcounter/counter");
    return response.data;
};

export const tryIncrementDownlinkCounter = async () => {
    const response = await apiClient.post("/downlinkcounter/try-increment");
    console.log(response);
    return response.data; // true oder false
};

export const deleteGate = async (gateId) => {
    return await apiClient.delete(`/gates/${gateId}`);
};

export const createGate = async (newGateData) => {
    return await apiClient.post("/add-gate-ui", newGateData);
};

export const sendDownlink = async (payload) => {
    return await apiClient.post("api/downlink", JSON.stringify(payload));
};

export const resetDownlinkCounter = async () => {
    return await apiClient.post("/downlinkcounter/reset");
};
