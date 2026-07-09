import { healthReceived } from '../slices/healthSlice';

const VALID_BATTERY_ENUMS = new Set([
  'CHARGING',
  'DISCHARGING',
  'LOW_BATTERY',
  'UNKNOWN',
]);

const VALID_SHOCK_ENUMS = new Set([
  'NO_SHOCK',
  'SHOCK_DETECTED',
  'UNKNOWN',
]);

const HEALTH_MESSAGE_TYPE = 5;

export function validateHealthPayload(payload) {
  const errors = [];

  if (
    payload === null ||
    typeof payload !== 'object' ||
    Array.isArray(payload)
  ) {
    return { valid: false, errors: ['Payload is not an object'], normalized: null };
  }

  if (payload.messageType !== HEALTH_MESSAGE_TYPE) {
    errors.push(
      `messageType must be ${HEALTH_MESSAGE_TYPE}, got: ${String(payload.messageType)}`
    );
    return { valid: false, errors, normalized: null };
  }

  if (!Array.isArray(payload.statuses)) {
    errors.push('statuses must be an array');
    return { valid: false, errors, normalized: null };
  }

  const normalized = [];

  for (const entry of payload.statuses) {
    if (
      entry === null ||
      typeof entry !== 'object' ||
      Array.isArray(entry)
    ) {
      errors.push('status entry is not an object, skipping');
      continue;
    }

    const rawId = entry.senseGateId;
    if (rawId === undefined || rawId === null) {
      continue;
    }

    const senseGateId = Number(rawId);
    if (Number.isNaN(senseGateId)) {
      continue;
    }

    normalized.push({
      senseGateId,
      batteryStatus: normalizeEnum(entry.batteryStatus, VALID_BATTERY_ENUMS),
      shockStatus: normalizeEnum(entry.shockStatus, VALID_SHOCK_ENUMS),
      voltageMv: coerceNumberOrNull(entry.voltageMv),
      version: coerceNumberOrNull(entry.version),
    });
  }

  return { valid: true, errors, normalized };
}

function normalizeEnum(raw, validEnums) {
  if (raw === undefined || raw === null) {
    return 'UNKNOWN';
  }
  const upper = String(raw).toUpperCase();
  return validEnums.has(upper) ? upper : 'UNKNOWN';
}

function coerceNumberOrNull(raw) {
  if (raw === undefined || raw === null) {
    return null;
  }
  const num = Number(raw);
  return Number.isNaN(num) ? null : num;
}

export function handleHealthMessage(rawBody, dispatch) {
  let parsed;
  try {
    parsed = JSON.parse(rawBody);
  } catch (err) {
    console.error('Failed to parse health message:', err);
    return;
  }

  const { valid, normalized } = validateHealthPayload(parsed);
  if (!valid || normalized === null || normalized.length === 0) {
    return;
  }

  dispatch(healthReceived({ statuses: normalized }));
}
