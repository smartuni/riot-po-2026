
import apiClient from '../../../shared/api/apiClient';

export const loadUserDetails = async () => {
    try {
        const response = await apiClient.get('/auth/user-details');
        if (response.status !== 200) {
            throw new Error('Request failed with status code ' + response.status);
        }
        return response.data;
    } catch (e) {
        console.error('Fehler beim Laden der User-Details:', e);
        throw e;
    }
};

export const loadWorkerId = async () => {
    const user = await loadUserDetails();
    return user.workerId;
};

export const loginUser = async (loginData) => {
    const response = await apiClient.post('/auth/login', loginData);
    return response.data;
};

export const registerUser = async (registerData) => {
    const response = await apiClient.post('/auth/register', registerData);
    return response.data;
};

export const updateUserDetails = async (request) => {
    const response = await apiClient.put('/auth/user-change', request);
    if (response.status !== 200) {
        throw new Error('Request failed with status code ' + response.status);
    }
    return response.data;
};

export const logoutUser = async () => {
    const response = await apiClient.post('/auth/logout');
    if (response.status !== 200) {
        throw new Error('Request failed with status code ' + response.status);
    }
    return response.data;
};
