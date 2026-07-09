import { describe, it, expect, afterEach, vi } from 'vitest';
import reducer, { healthReceived, resetHealth } from './healthSlice';

describe('healthSlice reducer', () => {
  afterEach(() => {
    vi.restoreAllMocks();
  });

  // Test 9: Initial state
  it('returns initial state { bySenseGateId: {} } when state is undefined', () => {
    const state = reducer(undefined, { type: 'unknown/action' });
    expect(state).toEqual({ bySenseGateId: {} });
  });

  // Test 10: healthReceived with new senseGateId
  it('healthReceived creates entry with per-field timestamps for new senseGateId', () => {
    const now = 1000;
    vi.spyOn(Date, 'now').mockReturnValue(now);

    const state = reducer(
      undefined,
      healthReceived({
        statuses: [
          {
            senseGateId: 1,
            batteryStatus: 'CHARGING',
            freeFallStatus: 'NO_FALL',
            shockStatus: 'NO_SHOCK',
            voltageMv: 3950,
            version: 3,
          },
        ],
      })
    );

    expect(state).toEqual({
      bySenseGateId: {
        1: {
          version: { value: 3, receivedAt: now },
          battery: { value: 'CHARGING', receivedAt: now },
          freeFall: { value: 'NO_FALL', receivedAt: now },
          shock: { value: 'NO_SHOCK', receivedAt: now },
          voltageMv: { value: 3950, receivedAt: now },
        },
      },
    });
  });

  // Test 11: free fall message (battery=UNKNOWN) -> battery PRESERVED, free fall UPDATED
  it('preserves battery value when incoming batteryStatus is UNKNOWN and previous exists', () => {
    const now1 = 1000;
    const now2 = 2000;

    const dateSpy = vi.spyOn(Date, 'now');
    dateSpy.mockReturnValue(now1);

    let state = reducer(
      undefined,
      healthReceived({
        statuses: [
          {
            senseGateId: 1,
            batteryStatus: 'CHARGING',
            freeFallStatus: 'NO_FALL',
            shockStatus: 'NO_SHOCK',
            voltageMv: 3950,
            version: 3,
          },
        ],
      })
    );

    dateSpy.mockReturnValue(now2);
    state = reducer(
      state,
      healthReceived({
        statuses: [
          {
            senseGateId: 1,
            batteryStatus: 'UNKNOWN',
            freeFallStatus: 'FREE_FALL_DETECTED',
            shockStatus: 'UNKNOWN',
            voltageMv: 0,
            version: 4,
          },
        ],
      })
    );

    expect(state.bySenseGateId[1].battery).toEqual({
      value: 'CHARGING',
      receivedAt: now1,
    });
    expect(state.bySenseGateId[1].freeFall).toEqual({
      value: 'FREE_FALL_DETECTED',
      receivedAt: now2,
    });
  });

  // Test 12: battery message (freeFall=UNKNOWN) -> free fall PRESERVED, battery UPDATED
  it('preserves free fall value when incoming freeFallStatus is UNKNOWN and previous exists', () => {
    const now1 = 1000;
    const now2 = 2000;

    const dateSpy = vi.spyOn(Date, 'now');
    dateSpy.mockReturnValue(now1);

    let state = reducer(
      undefined,
      healthReceived({
        statuses: [
          {
            senseGateId: 1,
            batteryStatus: 'CHARGING',
            freeFallStatus: 'NO_FALL',
            shockStatus: 'NO_SHOCK',
            voltageMv: 3950,
            version: 3,
          },
        ],
      })
    );

    dateSpy.mockReturnValue(now2);
    state = reducer(
      state,
      healthReceived({
        statuses: [
          {
            senseGateId: 1,
            batteryStatus: 'LOW_BATTERY',
            freeFallStatus: 'UNKNOWN',
            shockStatus: 'UNKNOWN',
            voltageMv: 0,
            version: 4,
          },
        ],
      })
    );

    expect(state.bySenseGateId[1].freeFall).toEqual({
      value: 'NO_FALL',
      receivedAt: now1,
    });
    expect(state.bySenseGateId[1].battery).toEqual({
      value: 'LOW_BATTERY',
      receivedAt: now2,
    });
  });

  // Test 13: voltageMv=0 after previous voltageMv=3950 -> previous PRESERVED
  it('preserves voltageMv when incoming value is 0 and previous non-zero exists', () => {
    const now1 = 1000;
    const now2 = 2000;

    const dateSpy = vi.spyOn(Date, 'now');
    dateSpy.mockReturnValue(now1);

    let state = reducer(
      undefined,
      healthReceived({
        statuses: [
          {
            senseGateId: 1,
            batteryStatus: 'CHARGING',
            freeFallStatus: 'NO_FALL',
            shockStatus: 'NO_SHOCK',
            voltageMv: 3950,
            version: 3,
          },
        ],
      })
    );

    dateSpy.mockReturnValue(now2);
    state = reducer(
      state,
      healthReceived({
        statuses: [
          {
            senseGateId: 1,
            batteryStatus: 'UNKNOWN',
            freeFallStatus: 'UNKNOWN',
            shockStatus: 'UNKNOWN',
            voltageMv: 0,
            version: 4,
          },
        ],
      })
    );

    expect(state.bySenseGateId[1].voltageMv).toEqual({
      value: 3950,
      receivedAt: now1,
    });
    expect(state.bySenseGateId[1].version).toEqual({
      value: 4,
      receivedAt: now2,
    });
  });

  // Test 14: resetHealth -> returns initial state
  it('resetHealth returns initial state', () => {
    vi.spyOn(Date, 'now').mockReturnValue(1000);

    let state = reducer(
      undefined,
      healthReceived({
        statuses: [
          {
            senseGateId: 1,
            batteryStatus: 'CHARGING',
            freeFallStatus: 'NO_FALL',
            shockStatus: 'NO_SHOCK',
            voltageMv: 3950,
            version: 3,
          },
        ],
      })
    );

    state = reducer(state, resetHealth());
    expect(state).toEqual({ bySenseGateId: {} });
  });

  // Additional: first value exception for battery=UNKNOWN
  it('accepts UNKNOWN as first battery value (first-value exception)', () => {
    const now = 1000;
    vi.spyOn(Date, 'now').mockReturnValue(now);

    const state = reducer(
      undefined,
      healthReceived({
        statuses: [
          {
            senseGateId: 5,
            batteryStatus: 'UNKNOWN',
            freeFallStatus: 'FREE_FALL_DETECTED',
            shockStatus: 'SHOCK_DETECTED',
            voltageMv: 0,
            version: 1,
          },
        ],
      })
    );

    expect(state.bySenseGateId[5].battery).toEqual({
      value: 'UNKNOWN',
      receivedAt: now,
    });
    expect(state.bySenseGateId[5].voltageMv).toEqual({
      value: 0,
      receivedAt: now,
    });
  });

  // Additional: multiple statuses in one action
  it('processes multiple statuses in a single healthReceived action', () => {
    const now = 1000;
    vi.spyOn(Date, 'now').mockReturnValue(now);

    const state = reducer(
      undefined,
      healthReceived({
        statuses: [
          {
            senseGateId: 1,
            batteryStatus: 'CHARGING',
            freeFallStatus: 'NO_FALL',
            shockStatus: 'NO_SHOCK',
            voltageMv: 3950,
            version: 1,
          },
          {
            senseGateId: 2,
            batteryStatus: 'DISCHARGING',
            freeFallStatus: 'FREE_FALL_DETECTED',
            shockStatus: 'SHOCK_DETECTED',
            voltageMv: 3700,
            version: 2,
          },
        ],
      })
    );

    expect(Object.keys(state.bySenseGateId)).toHaveLength(2);
    expect(state.bySenseGateId[1].battery).toEqual({
      value: 'CHARGING',
      receivedAt: now,
    });
    expect(state.bySenseGateId[2].battery).toEqual({
      value: 'DISCHARGING',
      receivedAt: now,
    });
  });
});
