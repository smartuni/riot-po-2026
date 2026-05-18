import { createSlice } from '@reduxjs/toolkit';

const initialState = {
  uplinkString: null,
};

const gatesSlice = createSlice({
  name: 'gates',
  initialState,
  reducers: {
    uplinkReceived(state, action) {
      state.uplinkString = action.payload;
    },
    resetUplinkString(state) {
      state.uplinkString = null;
    },
  },
});

export const { uplinkReceived, resetUplinkString } = gatesSlice.actions;
export default gatesSlice.reducer;
