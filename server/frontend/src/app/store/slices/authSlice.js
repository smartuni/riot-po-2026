import { createSlice, createAsyncThunk } from '@reduxjs/toolkit';
import { fetchBaseQuery } from '@reduxjs/toolkit/query/react';
import { getCookie } from '../../../shared/utils/cookie';

const baseQuery = fetchBaseQuery({
  baseUrl: import.meta.env.VITE_API_BASE_URL ?? '',
  credentials: 'include',
  prepareHeaders: (headers) => {
    const csrfToken = getCookie('XSRF-TOKEN');
    if (csrfToken) {
      headers.set('X-XSRF-TOKEN', csrfToken);
    }
    headers.set('Content-Type', 'application/json');
    return headers;
  },
});

export const initializeAuth = createAsyncThunk(
  'auth/initialize',
  async () => {
    const result = await baseQuery('/api/auth/user-details', {}, {});
    if (result.error) {
      throw new Error('Unauthorized');
    }
    return result.data;
  }
);

const initialState = {
  user: null,
  status: 'loading',    // 'loading' | 'authenticated' | 'unauthenticated'
  error: null,
};

const authSlice = createSlice({
  name: 'auth',
  initialState,
  reducers: {
    setUser(state, action) {
      state.user = action.payload;
      state.status = 'authenticated';
      state.error = null;
    },
    clearAuth(state) {
      state.user = null;
      state.status = 'unauthenticated';
      state.error = null;
    },
    setAuthError(state, action) {
      state.user = null;
      state.status = 'unauthenticated';
      state.error = action.payload;
    },
  },
  extraReducers: (builder) => {
    builder
      .addCase(initializeAuth.pending, (state) => {
        state.status = 'loading';
      })
      .addCase(initializeAuth.fulfilled, (state, action) => {
        state.user = action.payload;
        state.status = 'authenticated';
        state.error = null;
      })
      .addCase(initializeAuth.rejected, (state, action) => {
        state.user = null;
        state.status = 'unauthenticated';
        state.error = action.error.message;
      });
  },
});

export const { setUser, clearAuth, setAuthError } = authSlice.actions;
export default authSlice.reducer;
