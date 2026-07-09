import { useAppSelector } from '../../../app/store';

// Returns the health entry for a given gate ID (matched by senseGateId === gateId)
// Returns null if no health data exists for this gate
export function useHealthForGate(gateId) {
  return useAppSelector((state) => state.health.bySenseGateId[gateId] ?? null);
}
