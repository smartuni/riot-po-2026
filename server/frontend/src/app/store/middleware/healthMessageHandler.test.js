import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { handleHealthMessage, validateHealthPayload } from './healthMessageHandler';
import { healthReceived } from '../slices/healthSlice';

describe('handleHealthMessage', () => {
  let mockDispatch;
  let consoleErrorSpy;

  beforeEach(() => {
    mockDispatch = vi.fn();
    consoleErrorSpy = vi.spyOn(console, 'error').mockImplementation(() => {});
  });

  afterEach(() => {
    vi.restoreAllMocks();
  });

  // Test 1: Valid payload with all fields -> dispatches correct action
  it('dispatches healthReceived with normalized statuses for a valid payload', () => {
    const payload = {
      messageType: 5,
      statuses: [
        {
          senseGateId: 1,
          batteryStatus: 'CHARGING',
          shockStatus: 'NO_SHOCK',
          voltageMv: 3950,
          version: 3,
        },
      ],
    };

    handleHealthMessage(JSON.stringify(payload), mockDispatch);

    expect(mockDispatch).toHaveBeenCalledTimes(1);
    expect(mockDispatch).toHaveBeenCalledWith(
      healthReceived({
        statuses: [
          {
            senseGateId: 1,
            batteryStatus: 'CHARGING',
            shockStatus: 'NO_SHOCK',
            voltageMv: 3950,
            version: 3,
          },
        ],
      })
    );
  });

  // Test 2: Malformed JSON -> no dispatch, console.error called
  it('does not dispatch and logs error for malformed JSON', () => {
    handleHealthMessage('{ invalid json }', mockDispatch);

    expect(mockDispatch).not.toHaveBeenCalled();
    expect(consoleErrorSpy).toHaveBeenCalled();
  });

  // Test 3: Missing statuses array -> no dispatch
  it('does not dispatch when statuses is missing', () => {
    const payload = { messageType: 5 };
    handleHealthMessage(JSON.stringify(payload), mockDispatch);

    expect(mockDispatch).not.toHaveBeenCalled();
  });

  // Test 4: Empty statuses array -> no dispatch (valid but no-op)
  it('does not dispatch when statuses array is empty', () => {
    const payload = { messageType: 5, statuses: [] };
    handleHealthMessage(JSON.stringify(payload), mockDispatch);

    expect(mockDispatch).not.toHaveBeenCalled();
  });

  // Test 5: Missing senseGateId -> that entry skipped, others processed
  it('skips entries without senseGateId but processes valid entries in the same message', () => {
    const payload = {
      messageType: 5,
      statuses: [
        {
          batteryStatus: 'CHARGING',
          shockStatus: 'NO_SHOCK',
          voltageMv: 3950,
          version: 3,
        },
        {
          senseGateId: 2,
          batteryStatus: 'DISCHARGING',
          shockStatus: 'SHOCK_DETECTED',
          voltageMv: 3800,
          version: 1,
        },
      ],
    };

    handleHealthMessage(JSON.stringify(payload), mockDispatch);

    expect(mockDispatch).toHaveBeenCalledTimes(1);
    expect(mockDispatch).toHaveBeenCalledWith(
      healthReceived({
        statuses: [
          {
            senseGateId: 2,
            batteryStatus: 'DISCHARGING',
            shockStatus: 'SHOCK_DETECTED',
            voltageMv: 3800,
            version: 1,
          },
        ],
      })
    );
  });

  // Test 6: messageType !== 5 -> no dispatch
  it('does not dispatch when messageType is not 5', () => {
    const payload = {
      messageType: 3,
      statuses: [
        {
          senseGateId: 1,
          batteryStatus: 'CHARGING',
          shockStatus: 'NO_SHOCK',
          voltageMv: 3950,
          version: 3,
        },
      ],
    };

    handleHealthMessage(JSON.stringify(payload), mockDispatch);

    expect(mockDispatch).not.toHaveBeenCalled();
  });

  // Test 7: Unknown enum value for batteryStatus -> normalized to UNKNOWN, dispatch proceeds
  it('normalizes unknown batteryStatus to UNKNOWN and still dispatches', () => {
    const payload = {
      messageType: 5,
      statuses: [
        {
          senseGateId: 1,
          batteryStatus: 'GARBAGE_VALUE',
          shockStatus: 'NO_SHOCK',
          voltageMv: 3950,
          version: 3,
        },
      ],
    };

    handleHealthMessage(JSON.stringify(payload), mockDispatch);

    expect(mockDispatch).toHaveBeenCalledTimes(1);
    const dispatched = mockDispatch.mock.calls[0][0];
    expect(dispatched.payload.statuses[0].batteryStatus).toBe('UNKNOWN');
  });

  // Test 8: Multiple valid entries -> all dispatched
  it('dispatches all valid entries in a single message', () => {
    const payload = {
      messageType: 5,
      statuses: [
        {
          senseGateId: 1,
          batteryStatus: 'CHARGING',
          shockStatus: 'NO_SHOCK',
          voltageMv: 3950,
          version: 1,
        },
        {
          senseGateId: 2,
          batteryStatus: 'DISCHARGING',
          shockStatus: 'SHOCK_DETECTED',
          voltageMv: 3700,
          version: 2,
        },
        {
          senseGateId: 3,
          batteryStatus: 'LOW_BATTERY',
          shockStatus: 'NO_SHOCK',
          voltageMv: 3500,
          version: 3,
        },
      ],
    };

    handleHealthMessage(JSON.stringify(payload), mockDispatch);

    expect(mockDispatch).toHaveBeenCalledTimes(1);
    const dispatched = mockDispatch.mock.calls[0][0];
    expect(dispatched.payload.statuses).toHaveLength(3);
    expect(dispatched.payload.statuses[0].senseGateId).toBe(1);
    expect(dispatched.payload.statuses[1].senseGateId).toBe(2);
    expect(dispatched.payload.statuses[2].senseGateId).toBe(3);
  });

  // Additional: string senseGateId is coerced to number
  it('coerces string senseGateId to number', () => {
    const payload = {
      messageType: 5,
      statuses: [
        {
          senseGateId: '42',
          batteryStatus: 'CHARGING',
          shockStatus: 'NO_SHOCK',
          voltageMv: 3950,
          version: 1,
        },
      ],
    };

    handleHealthMessage(JSON.stringify(payload), mockDispatch);

    expect(mockDispatch).toHaveBeenCalledTimes(1);
    const dispatched = mockDispatch.mock.calls[0][0];
    expect(dispatched.payload.statuses[0].senseGateId).toBe(42);
    expect(typeof dispatched.payload.statuses[0].senseGateId).toBe('number');
  });

  // Additional: lowercase enum is normalized to uppercase
  it('normalizes lowercase enum values to uppercase', () => {
    const payload = {
      messageType: 5,
      statuses: [
        {
          senseGateId: 1,
          batteryStatus: 'charging',
          shockStatus: 'no_shock',
          voltageMv: 3950,
          version: 1,
        },
      ],
    };

    handleHealthMessage(JSON.stringify(payload), mockDispatch);

    const dispatched = mockDispatch.mock.calls[0][0];
    expect(dispatched.payload.statuses[0].batteryStatus).toBe('CHARGING');
    expect(dispatched.payload.statuses[0].shockStatus).toBe('NO_SHOCK');
  });

  // Additional: missing voltageMv and version -> null in normalized output
  it('normalizes absent voltageMv and version to null', () => {
    const payload = {
      messageType: 5,
      statuses: [
        {
          senseGateId: 1,
          batteryStatus: 'CHARGING',
          shockStatus: 'NO_SHOCK',
        },
      ],
    };

    handleHealthMessage(JSON.stringify(payload), mockDispatch);

    const dispatched = mockDispatch.mock.calls[0][0];
    expect(dispatched.payload.statuses[0].voltageMv).toBeNull();
    expect(dispatched.payload.statuses[0].version).toBeNull();
  });
});

describe('validateHealthPayload', () => {
  it('returns valid result for correct messageType and statuses array', () => {
    const payload = {
      messageType: 5,
      statuses: [
        {
          senseGateId: 1,
          batteryStatus: 'CHARGING',
          shockStatus: 'NO_SHOCK',
          voltageMv: 3950,
          version: 3,
        },
      ],
    };

    const result = validateHealthPayload(payload);

    expect(result.valid).toBe(true);
    expect(result.errors).toHaveLength(0);
    expect(result.normalized).toHaveLength(1);
    expect(result.normalized[0].senseGateId).toBe(1);
  });

  it('returns invalid result when messageType is not 5', () => {
    const result = validateHealthPayload({ messageType: 3, statuses: [] });

    expect(result.valid).toBe(false);
    expect(result.normalized).toBeNull();
    expect(result.errors.length).toBeGreaterThan(0);
  });

  it('returns invalid result when statuses is not an array', () => {
    const result = validateHealthPayload({ messageType: 5, statuses: 'not-an-array' });

    expect(result.valid).toBe(false);
    expect(result.normalized).toBeNull();
  });

  it('returns valid with empty normalized array when statuses is empty', () => {
    const result = validateHealthPayload({ messageType: 5, statuses: [] });

    expect(result.valid).toBe(true);
    expect(result.normalized).toEqual([]);
  });

  it('skips entries without a valid senseGateId', () => {
    const payload = {
      messageType: 5,
      statuses: [
        { batteryStatus: 'CHARGING', shockStatus: 'NO_SHOCK' },
        { senseGateId: 'abc', batteryStatus: 'CHARGING', shockStatus: 'NO_SHOCK' },
        { senseGateId: 5, batteryStatus: 'CHARGING', shockStatus: 'NO_SHOCK' },
      ],
    };

    const result = validateHealthPayload(payload);

    expect(result.valid).toBe(true);
    expect(result.normalized).toHaveLength(1);
    expect(result.normalized[0].senseGateId).toBe(5);
  });
});
