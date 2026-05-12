import { configureStore } from '@reduxjs/toolkit';
import { useDispatch, useSelector } from 'react-redux';
import { getCookie } from '../../shared/utils/cookie';

const initialAuthState = {
  token: getCookie('jwt') || null,
};

function authReducer(state = initialAuthState, action) {
  switch (action.type) {
    case 'auth/setToken':
      return { ...state, token: action.payload };
    case 'auth/clearToken':
      return { ...state, token: null };
    default:
      return state;
  }
}

export const store = configureStore({
  reducer: {
    auth: authReducer,
  },
});

export const useAppDispatch = () => useDispatch();
export const useAppSelector = (selector) => useSelector(selector);

export default store;
