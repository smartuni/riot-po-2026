import React from 'react';
import { batteryInfo, freeFallInfo, voltageInfo, isStale } from '../healthUtils';

const PULSE_STYLE = `
@keyframes health-pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.4; }
}
.health-pulse-icon {
  animation: health-pulse 1.5s ease-in-out infinite;
}
@media (prefers-reduced-motion: reduce) {
  .health-pulse-icon {
    animation: none;
  }
}
`;

const indicatorStyle = (color) => ({
  display: 'inline-flex',
  alignItems: 'center',
  gap: '4px',
  fontSize: '13px',
  fontWeight: 500,
  color,
});

const dotStyle = (color) => ({
  width: '8px',
  height: '8px',
  borderRadius: '50%',
  background: color,
  display: 'inline-block',
  flexShrink: 0,
});

const HealthBadge = ({ health }) => {
  if (!health) {
    return (
      <>
        <style>{PULSE_STYLE}</style>
        <div
          role="img"
          aria-label="No health data available"
          data-testid="health-badge"
          style={{ fontSize: '13px', color: 'var(--text-secondary)' }}
        >
          Awaiting first health report
        </div>
      </>
    );
  }

  const { battery, freeFall, voltageMv } = health;

  const batt = batteryInfo(battery?.value);
  const sInfo = freeFallInfo(freeFall?.value);
  const volt = voltageInfo(voltageMv?.value);

  const isAnyStale = isStale(battery?.receivedAt) || isStale(freeFall?.receivedAt) || isStale(voltageMv?.receivedAt);
  const voltageText = volt.display !== '—' ? `${volt.display}${volt.unit}` : '—';

  const ariaLabel = `Battery: ${batt.label}, Free Fall: ${sInfo.label}, Voltage: ${voltageText}`;

  return (
    <>
      <style>{PULSE_STYLE}</style>
      <div
        role="img"
        aria-label={ariaLabel}
        data-testid="health-badge"
        data-stale={isAnyStale}
        style={{
          display: 'flex',
          alignItems: 'center',
          gap: '12px',
          fontSize: '13px',
          flexWrap: 'wrap',
        }}
      >
        <span style={indicatorStyle(batt.color)} data-testid="health-battery">
          <span style={dotStyle(batt.color)} />
          {batt.label}
        </span>
        <span style={indicatorStyle(sInfo.color)} data-testid="health-freefall">
          <span style={dotStyle(sInfo.color)} className={sInfo.pulse ? 'health-pulse-icon' : ''} />
          {sInfo.label}
        </span>
        <span
          data-testid="health-voltage"
          style={{ fontFamily: 'monospace', fontSize: '13px', color: 'var(--text)' }}
        >
          {voltageText}
        </span>
      </div>
    </>
  );
};

export default HealthBadge;
