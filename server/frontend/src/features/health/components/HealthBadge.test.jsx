import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest';
import { render, screen, within } from '@testing-library/react';
import HealthBadge from './HealthBadge';
import { STALE_THRESHOLD_MS } from '../healthUtils';

// Helper: build a fresh health entry
const freshHealth = (overrides = {}) => {
  const now = Date.now();
  return {
    version: { value: 1, receivedAt: now },
    battery: { value: 'CHARGING', receivedAt: now },
    shock: { value: 'NO_SHOCK', receivedAt: now },
    voltageMv: { value: 3950, receivedAt: now },
    ...overrides,
  };
};

describe('HealthBadge', () => {
  beforeEach(() => {
    // Freeze "now" so staleness tests are deterministic
    vi.spyOn(Date, 'now').mockReturnValue(1_000_000);
  });

  afterEach(() => {
    vi.restoreAllMocks();
  });

  // 1. Renders battery icon with correct color for CHARGING
  it('renders battery icon with green color when battery status is CHARGING', () => {
    const health = freshHealth({ battery: { value: 'CHARGING', receivedAt: Date.now() } });
    render(<HealthBadge health={health} />);

    const badge = screen.getByTestId('health-badge');
    const batteryIcon = within(badge).getByTestId('health-battery');
    expect(batteryIcon).toBeInTheDocument();
    // CSS variable for green-600 used by batteryInfo('CHARGING')
    expect(batteryIcon.getAttribute('style')).toContain('var(--green-600)');
  });

  // 2. Renders battery icon with red color for LOW_BATTERY
  it('renders battery icon with red color when battery status is LOW_BATTERY', () => {
    const health = freshHealth({ battery: { value: 'LOW_BATTERY', receivedAt: Date.now() } });
    render(<HealthBadge health={health} />);

    const batteryIcon = screen.getByTestId('health-battery');
    expect(batteryIcon).toBeInTheDocument();
    expect(batteryIcon.getAttribute('style')).toContain('var(--red-600)');
  });

  // 3. Renders shock with pulse class for SHOCK_DETECTED
  it('adds pulse class to shock dot when shock status is SHOCK_DETECTED', () => {
    const health = freshHealth({ shock: { value: 'SHOCK_DETECTED', receivedAt: Date.now() } });
    render(<HealthBadge health={health} />);

    const shockEl = screen.getByTestId('health-shock');
    const dot = shockEl.querySelector('.health-pulse-icon');
    expect(dot).not.toBeNull();
  });

  // 4. Renders shock without pulse class for NO_SHOCK
  it('does not add pulse class to shock dot when shock status is NO_SHOCK', () => {
    const health = freshHealth({ shock: { value: 'NO_SHOCK', receivedAt: Date.now() } });
    render(<HealthBadge health={health} />);

    const shockEl = screen.getByTestId('health-shock');
    const dot = shockEl.querySelector('.health-pulse-icon');
    expect(dot).toBeNull();
  });

  // 5. Renders empty state with "Awaiting first health report" when health is null
  it('renders empty state message when health is null', () => {
    render(<HealthBadge health={null} />);

    expect(screen.getByText('Awaiting first health report')).toBeInTheDocument();
    expect(screen.getByTestId('health-badge')).toHaveAttribute(
      'aria-label',
      'No health data available'
    );
  });

  // 6. Renders "—" for voltage when voltageMv is 0 or null
  it('renders em dash for voltage when voltageMv value is 0', () => {
    const health = freshHealth({ voltageMv: { value: 0, receivedAt: Date.now() } });
    render(<HealthBadge health={health} />);

    const voltage = screen.getByTestId('health-voltage');
    expect(voltage.textContent).toBe('—');
  });

  it('renders em dash for voltage when voltageMv is null', () => {
    const health = freshHealth({ voltageMv: null });
    render(<HealthBadge health={health} />);

    const voltage = screen.getByTestId('health-voltage');
    expect(voltage.textContent).toBe('—');
  });

  // 7. Renders voltage in volts (e.g. "3.95V") when voltageMv is 3950
  it('renders voltage formatted in volts when voltageMv is 3950', () => {
    const health = freshHealth({ voltageMv: { value: 3950, receivedAt: Date.now() } });
    render(<HealthBadge health={health} />);

    const voltage = screen.getByTestId('health-voltage');
    expect(voltage.textContent).toBe('3.95V');
  });

  // 8. Has role="img" and descriptive aria-label
  it('has role="img" and a descriptive aria-label including battery, shock, and voltage', () => {
    const health = freshHealth({
      battery: { value: 'CHARGING', receivedAt: Date.now() },
      shock: { value: 'NO_SHOCK', receivedAt: Date.now() },
      voltageMv: { value: 3950, receivedAt: Date.now() },
    });
    render(<HealthBadge health={health} />);

    const badge = screen.getByRole('img');
    expect(badge).toBeInTheDocument();
    const aria = badge.getAttribute('aria-label');
    expect(aria).toContain('Battery: Charging');
    expect(aria).toContain('Shock: No Shock');
    expect(aria).toContain('Voltage: 3.95V');
  });

  // 9. Applies stale styling when data is older than STALE_THRESHOLD_MS
  it('applies stale styling when data is older than STALE_THRESHOLD_MS', () => {
    const now = 1_000_000;
    const staleReceivedAt = now - STALE_THRESHOLD_MS - 1;
    const health = {
      battery: { value: 'CHARGING', receivedAt: staleReceivedAt },
      shock: { value: 'NO_SHOCK', receivedAt: staleReceivedAt },
      voltageMv: { value: 3950, receivedAt: staleReceivedAt },
    };
    render(<HealthBadge health={health} />);

    const badge = screen.getByTestId('health-badge');
    expect(badge.getAttribute('data-stale')).toBe('true');
  });

  it('does not apply stale styling when data is fresh', () => {
    const health = freshHealth();
    render(<HealthBadge health={health} />);

    const badge = screen.getByTestId('health-badge');
    expect(badge.getAttribute('data-stale')).toBe('false');
  });

  // 10. Renders all indicators together when all fields have values
  it('renders battery, shock, and voltage indicators together when all fields have values', () => {
    const health = freshHealth({
      battery: { value: 'DISCHARGING', receivedAt: Date.now() },
      shock: { value: 'SHOCK_DETECTED', receivedAt: Date.now() },
      voltageMv: { value: 3700, receivedAt: Date.now() },
    });
    render(<HealthBadge health={health} />);

    const badge = screen.getByTestId('health-badge');
    expect(within(badge).getByTestId('health-battery')).toBeInTheDocument();
    expect(within(badge).getByTestId('health-shock')).toBeInTheDocument();
    expect(within(badge).getByTestId('health-voltage')).toBeInTheDocument();

    // Verify correct values for each indicator
    const battery = within(badge).getByTestId('health-battery');
    expect(battery.getAttribute('style')).toContain('var(--blue-500)');

    const shock = within(badge).getByTestId('health-shock');
    expect(shock.querySelector('.health-pulse-icon')).not.toBeNull();
    expect(shock.getAttribute('style')).toContain('var(--red-600)');

    const voltage = within(badge).getByTestId('health-voltage');
    expect(voltage.textContent).toBe('3.70V');
  });
});
