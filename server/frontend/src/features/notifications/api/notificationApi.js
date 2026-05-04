import apiClient from '../../../shared/api/apiClient';

export const fetchNotificationByWorkerId = async (workerId) => {
    try {
        const response = await apiClient.get(`/notifications/${workerId}`);
        return response.data;
    } catch (error) {
        console.error('Error fetching notifications:', error);
        throw error;
    }
};

export const fetchNotification = async () => {
    try {
        const response = await apiClient.get('/notifications');
        return response.data;
    } catch (error) {
        console.error('Error fetching notifications:', error);
        throw error;
    }
};

export const markNotificationAsRead = async (notificationId) => {
    try {
        console.log("Markiere Notification als gelesen:", notificationId); // zum Debuggen
        await apiClient.post(`/notifications/${notificationId}/request-read-change`);
    } catch (error) {
        console.error("Fehler beim Aktualisieren der Benachrichtigung:", error);
        throw error;
    }
};
