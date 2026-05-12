import { configureStore } from '@reduxjs/toolkit';
import { useDispatch, useSelector } from 'react-redux';
import { api } from './api/api';
import authReducer from './slices/authSlice';
import gatesReducer from './slices/gatesSlice';
import activitiesReducer from './slices/activitiesSlice';
import notificationsReducer from './slices/notificationsSlice';
import downlinkReducer from './slices/downlinkSlice';
import wsMiddleware from './middleware/wsMiddleware';

export const store = configureStore({
  reducer: {
    auth: authReducer,
    gates: gatesReducer,
    activities: activitiesReducer,
    notifications: notificationsReducer,
    downlink: downlinkReducer,
    [api.reducerPath]: api.reducer,
  },
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware().concat(api.middleware, wsMiddleware),
});

export const useAppDispatch = () => useDispatch();
export const useAppSelector = (selector) => useSelector(selector);

export default store;