import { createSlice } from '@reduxjs/toolkit';

const initialState = {
  gates: [],
  uplinkString: null,
};

const gatesSlice = createSlice({
  name: 'gates',
  initialState,
  reducers: {
    gateAdded(state, action) {
      state.gates.push(action.payload);
    },
    gateDeleted(state, action) {
      const id = parseInt(action.payload);
      state.gates = state.gates.filter((gate) => gate.id !== id);
    },
    gateUpdated(state, action) {
      const index = state.gates.findIndex(
        (gate) => gate.id === action.payload.id
      );
      if (index !== -1) {
        state.gates[index] = action.payload;
      }
    },
    uplinkReceived(state, action) {
      state.uplinkString = action.payload;
    },
  },
});

export const { gateAdded, gateDeleted, gateUpdated, uplinkReceived } =
  gatesSlice.actions;
export default gatesSlice.reducer;
