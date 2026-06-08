import { createSlice, createAsyncThunk } from '@reduxjs/toolkit';

export const initializeAuth = createAsyncThunk(
  'auth/initialize',
  async () => {
    const response = await fetch('/api/auth/user-details', {
      credentials: 'include',
    });
    if (!response.ok) {
      throw new Error('Unauthorized');
    }
    return await response.json();
  }
);

const initialState = {
  user: null,
  token: null,
  status: 'loading',    // 'loading' | 'authenticated' | 'unauthenticated'
  error: null,
};

const authSlice = createSlice({
  name: 'auth',
  initialState,
  reducers: {
    setUser(state, action) {
      state.user = action.payload;
      state.token = action.payload?.token ?? null;
      state.status = 'authenticated';
      state.error = null;
    },
    clearAuth(state) {
      state.user = null;
      state.token = null;
      state.status = 'unauthenticated';
      state.error = null;
    },
    setAuthError(state, action) {
      state.user = null;
      state.token = null;
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
