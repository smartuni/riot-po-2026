import { configureStore } from '@reduxjs/toolkit';
import { useDispatch, useSelector } from 'react-redux';
import { api } from './api/api';
import authReducer from './slices/authSlice';
import gatesReducer from './slices/gatesSlice';
import uiReducer from './slices/uiSlice';
import wsMiddleware from './middleware/wsMiddleware';

export const store = configureStore({
  reducer: {
    auth: authReducer,
    gates: gatesReducer,
    ui: uiReducer,
    [api.reducerPath]: api.reducer,
  },
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware().concat(api.middleware, wsMiddleware),
});

export const useAppDispatch = () => useDispatch();
export const useAppSelector = useSelector;

export default store;
