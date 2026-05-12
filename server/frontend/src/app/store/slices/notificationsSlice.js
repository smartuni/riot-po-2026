import { createSlice } from '@reduxjs/toolkit';

const initialState = {
  notifications: [],
};

const notificationsSlice = createSlice({
  name: 'notifications',
  initialState,
  reducers: {
    notificationsLoaded(state, action) {
      state.notifications = action.payload;
    },
    notificationMarkedRead(state, action) {
      const notification = state.notifications.find(
        (n) => n.id === action.payload
      );
      if (notification) {
        notification.isRead = true;
      }
    },
  },
});

export const { notificationsLoaded, notificationMarkedRead } =
  notificationsSlice.actions;
export default notificationsSlice.reducer;
