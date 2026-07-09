import { createSlice } from '@reduxjs/toolkit';

const initialState = {
  bySenseGateId: {},
};

const SENTINEL_VALUES = {
  battery: 'UNKNOWN',
  freeFall: 'UNKNOWN',
  voltageMv: 0,
};

function isSentinelValue(fieldName, value) {
  const sentinel = SENTINEL_VALUES[fieldName];
  return sentinel !== undefined && value === sentinel;
}

function mergeField(currentEntry, fieldName, incomingValue, now) {
  const existing = currentEntry[fieldName];

  if (existing === undefined || existing === null) {
    return { value: incomingValue, receivedAt: now };
  }

  if (isSentinelValue(fieldName, incomingValue)) {
    return existing;
  }

  return { value: incomingValue, receivedAt: now };
}

function mergeStatusIntoEntry(state, status, now) {
  const entry = state[status.senseGateId] ?? {};

  if (status.batteryStatus !== null && status.batteryStatus !== undefined) {
    entry.battery = mergeField(entry, 'battery', status.batteryStatus, now);
  }
  if (status.freeFallStatus !== null && status.freeFallStatus !== undefined) {
    entry.freeFall = mergeField(entry, 'freeFall', status.freeFallStatus, now);
  }
  if (status.voltageMv !== null && status.voltageMv !== undefined) {
    entry.voltageMv = mergeField(entry, 'voltageMv', status.voltageMv, now);
  }
  if (status.version !== null && status.version !== undefined) {
    entry.version = mergeField(entry, 'version', status.version, now);
  }

  state[status.senseGateId] = entry;
}

const healthSlice = createSlice({
  name: 'health',
  initialState,
  reducers: {
    healthReceived(state, action) {
      const { statuses } = action.payload;
      if (!Array.isArray(statuses)) return;

      const now = Date.now();
      for (const status of statuses) {
        if (
          status === null ||
          typeof status !== 'object' ||
          status.senseGateId === undefined ||
          status.senseGateId === null ||
          typeof status.senseGateId !== 'number' ||
          Number.isNaN(status.senseGateId)
        ) {
          continue;
        }

        mergeStatusIntoEntry(state.bySenseGateId, status, now);
      }
    },
    resetHealth() {
      return initialState;
    },
  },
});

export const { healthReceived, resetHealth } = healthSlice.actions;
export default healthSlice.reducer;
