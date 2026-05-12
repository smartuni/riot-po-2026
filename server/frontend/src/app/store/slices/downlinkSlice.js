import { createSlice } from '@reduxjs/toolkit';

const initialState = {
  count: 0,
};

const downlinkSlice = createSlice({
  name: 'downlink',
  initialState,
  reducers: {
    counterLoaded(state, action) {
      state.count = action.payload;
    },
    counterReset(state) {
      state.count = 0;
    },
  },
});

export const { counterLoaded, counterReset } = downlinkSlice.actions;
export default downlinkSlice.reducer;
