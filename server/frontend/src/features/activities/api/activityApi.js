import apiClient from '../../../shared/api/apiClient';

export const fetchActivities = async () => {
    try {
        const response = await apiClient.get('/gate-activities');
        return response.data;
    } catch (error) {
        console.error('Error fetching gate-activities:', error);
        throw error;
    }
};

export const addActivities = async (newActivities) => {
    try {
        const response = await apiClient.post(`/add-activities/`, newActivities);
        return response.data;
    } catch (error) {
        console.error('Error adding activities:', error);
        throw error;
    }
}
