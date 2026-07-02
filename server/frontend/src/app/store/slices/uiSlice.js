import { createSlice } from '@reduxjs/toolkit';

const DARK_KEY = 'sensemante-dark';

function readSessionDark() {
  try {
    const saved = sessionStorage.getItem(DARK_KEY);
    if (saved !== null) return saved === 'true';
  } catch {
    // sessionStorage unavailable (SSR / test env)
  }
  return window.matchMedia('(prefers-color-scheme: dark)').matches;
}

const uiSlice = createSlice({
  name: 'ui',
  initialState: {
    darkMode: readSessionDark(),
  },
  reducers: {
    toggleDarkMode(state) {
      state.darkMode = !state.darkMode;
    },
    setDarkMode(state, action) {
      state.darkMode = action.payload;
    },
  },
});

export const { toggleDarkMode, setDarkMode } = uiSlice.actions;
export default uiSlice.reducer;
