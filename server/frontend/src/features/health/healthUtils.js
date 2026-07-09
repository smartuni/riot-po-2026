// Stale threshold — placeholder, TODO: tune with firmware broadcast cadence
export const STALE_THRESHOLD_MS = 5 * 60 * 1000; // 5 minutes

// BatteryStatus enum constants
export const BATTERY_STATUS = {
  CHARGING: 'CHARGING',
  DISCHARGING: 'DISCHARGING',
  LOW_BATTERY: 'LOW_BATTERY',
  UNKNOWN: 'UNKNOWN',
};

// FreeFallStatus enum constants
export const FREE_FALL_STATUS = {
  NO_FALL: 'NO_FALL',
  FREE_FALL_DETECTED: 'FREE_FALL_DETECTED',
  UNKNOWN: 'UNKNOWN',
};

// ShockStatus enum constants
export const SHOCK_STATUS = {
  NO_SHOCK: 'NO_SHOCK',
  SHOCK_DETECTED: 'SHOCK_DETECTED',
  UNKNOWN: 'UNKNOWN',
};

// Returns { icon, color, label } for a battery status value
export function batteryInfo(value) {
  switch (value) {
    case 'CHARGING':
      return { icon: 'BatteryChargingFull', color: 'var(--green-600)', label: 'Charging' };
    case 'DISCHARGING':
      return { icon: 'BatteryFull', color: 'var(--blue-500)', label: 'Discharging' };
    case 'LOW_BATTERY':
      return { icon: 'BatteryAlert', color: 'var(--red-600)', label: 'Low Battery' };
    case 'UNKNOWN':
      return { icon: 'BatteryUnknown', color: 'var(--slate-400)', label: 'Unknown' };
    default:
      return { icon: 'BatteryUnknown', color: 'var(--slate-400)', label: '—' };
  }
}

// Returns { icon, color, label, pulse } for a free fall status value
export function freeFallInfo(value) {
  switch (value) {
    case 'NO_FALL':
      return { icon: 'ArrowDropDown', color: 'var(--blue-500)', label: 'No Free Fall', pulse: false };
    case 'FREE_FALL_DETECTED':
      return { icon: 'ArrowDropDown', color: 'var(--red-600)', label: 'Free Fall Detected', pulse: true };
    case 'UNKNOWN':
      return { icon: 'ArrowDropDown', color: 'var(--slate-400)', label: 'Unknown', pulse: false };
    default:
      return { icon: 'ArrowDropDown', color: 'var(--slate-400)', label: '—', pulse: false };
  }
}

// Returns { icon, color, label, pulse } for a shock status value
export function shockInfo(value) {
  switch (value) {
    case 'NO_SHOCK':
      return { icon: 'Vibration', color: 'var(--blue-500)', label: 'No Shock', pulse: false };
    case 'SHOCK_DETECTED':
      return { icon: 'Vibration', color: 'var(--red-600)', label: 'Shock Detected', pulse: true };
    case 'UNKNOWN':
      return { icon: 'Vibration', color: 'var(--slate-400)', label: 'Unknown', pulse: false };
    default:
      return { icon: 'Vibration', color: 'var(--slate-400)', label: '—', pulse: false };
  }
}

// Returns { display, unit } for a voltage value
export function voltageInfo(value) {
  if (value == null || value === 0) return { display: '—', unit: '' };
  return { display: (value / 1000).toFixed(2), unit: 'V' };
}

// Returns true if receivedAt is older than STALE_THRESHOLD_MS
export function isStale(receivedAt, now = Date.now()) {
  if (receivedAt == null) return false;
  return (now - receivedAt) > STALE_THRESHOLD_MS;
}

// Returns { hasAlert, alertCount, summaryText } for an aggregate health entry
// (used by dashboard summary)
export function getHealthSummary(healthEntry) {
  if (!healthEntry) return { hasAlert: false, alertCount: 0, summaryText: 'No health data' };
  let alerts = 0;
  if (healthEntry.battery?.value === 'LOW_BATTERY') alerts++;
  if (healthEntry.freeFall?.value === 'FREE_FALL_DETECTED') alerts++;
  if (healthEntry.shock?.value === 'SHOCK_DETECTED') alerts++;
  return {
    hasAlert: alerts > 0,
    alertCount: alerts,
    summaryText: alerts === 0 ? 'Healthy' : `${alerts} alert${alerts > 1 ? 's' : ''}`,
  };
}
