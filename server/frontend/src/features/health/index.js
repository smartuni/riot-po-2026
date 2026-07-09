export { default as HealthBadge } from './components/HealthBadge';
export { useHealthForGate } from './hooks/useHealthForGate';
export {
  batteryInfo,
  shockInfo,
  voltageInfo,
  isStale,
  getHealthSummary,
  STALE_THRESHOLD_MS,
  BATTERY_STATUS,
  SHOCK_STATUS,
} from './healthUtils';
