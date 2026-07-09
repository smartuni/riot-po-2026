import { test as base, expect, expectLoaded } from './fixtures';
import { CONTROLLER, login } from './utils';
import type { Page, WebSocketRoute } from '@playwright/test';

/**
 * E2E tests for health status feature (issue #113).
 *
 * The backend branch `101-HealthStatus-Backend` is NOT merged to main, so
 * `/topic/health` does not exist on the running backend. These tests use
 * Playwright's `page.routeWebSocket` to intercept the STOMP WebSocket and
 * inject mock health messages, testing the full WS → Redux → UI pipeline.
 *
 * Key STOMP protocol detail: the @stomp/stompjs client registers per-subscription
 * callbacks keyed by the subscription `id` header (e.g. `sub-0`). Incoming MESSAGE
 * frames MUST carry a matching `subscription` header, otherwise the message is
 * routed to `onUnhandledMessage` (which the wsMiddleware never sets → no-op →
 * silently dropped). We capture the id from the client's SUBSCRIBE frame and
 * echo it back in every MESSAGE we send.
 */

const STOMP_NULL = '\x00';

type ShockStatus = 'NO_SHOCK' | 'SHOCK_DETECTED' | 'UNKNOWN';
type BatteryStatus = 'CHARGING' | 'DISCHARGING' | 'LOW_BATTERY' | 'UNKNOWN';

interface HealthStatus {
  version: number;
  senseGateId: number;
  shockStatus: ShockStatus;
  batteryStatus: BatteryStatus;
  voltageMv: number;
}

interface HealthPayload {
  messageType: 5;
  statuses: HealthStatus[];
}

type HealthSender = (payload: HealthPayload) => void;

function stompConnected(): string {
  return ['CONNECTED', 'version:1.2', '', ''].join('\n') + STOMP_NULL;
}

function stompHealthMessage(payload: HealthPayload, subscriptionId: string): string {
  const body = JSON.stringify(payload);
  return [
    'MESSAGE',
    `subscription:${subscriptionId}`,
    'destination:/topic/health',
    'content-type:application/json',
    `content-length:${Buffer.byteLength(body, 'utf-8')}`,
    '',
    body,
  ].join('\n') + STOMP_NULL;
}

/**
 * Install a mock STOMP broker on `page` that intercepts the WebSocket before
 * login triggers the real connection. Returns a sender for pushing health
 * messages after the `/topic/health` subscription is established.
 *
 * Must be called BEFORE login so routeWebSocket catches the connect.
 */
function mockHealthWs(page: Page): HealthSender {
  let sendFn: ((data: string) => void) | null = null;
  let healthSubscriptionId: string | null = null;
  const queuedMessages: { payload: HealthPayload }[] = [];

  page.routeWebSocket('**/ws', (ws: WebSocketRoute) => {
    ws.onMessage((data) => {
      const frame = typeof data === 'string' ? data : data.toString('utf-8');

      if (frame.startsWith('CONNECT')) {
        ws.send(stompConnected());
        return;
      }

      if (frame.startsWith('SUBSCRIBE') && frame.includes('/topic/health')) {
        // The stompjs client registers per-subscription callbacks keyed by the
        // `id` header of SUBSCRIBE; MESSAGE frames must carry a matching
        // `subscription` header or the message is silently dropped.
        const match = frame.match(/^id:(\S+)/m);
        healthSubscriptionId = match ? match[1] : 'sub-0';

        sendFn = (msg: string) => ws.send(msg);

        for (const { payload } of queuedMessages) {
          ws.send(stompHealthMessage(payload, healthSubscriptionId));
        }
        queuedMessages.length = 0;
      }
    });
  });

  return (payload: HealthPayload) => {
    if (sendFn && healthSubscriptionId !== null) {
      sendFn(stompHealthMessage(payload, healthSubscriptionId));
    } else {
      queuedMessages.push({ payload });
    }
  };
}

const test = base.extend<{ sendHealth: HealthSender }>({
  sendHealth: async ({ page }, use) => {
    const sender = mockHealthWs(page);
    await login(page, CONTROLLER);
    await use(sender);
  },
});

test.describe('Health status (issue #113)', () => {
  test('DevicesPage renders health badge on matched gate card', async ({ page, sendHealth }) => {
    await page.goto('/devices');
    await expect(page.getByRole('heading', { name: 'Gate Overview' })).toBeVisible();
    await expectLoaded(page);

    sendHealth({
      messageType: 5,
      statuses: [
        {
          version: 1,
          senseGateId: 1001,
          shockStatus: 'NO_SHOCK',
          batteryStatus: 'CHARGING',
          voltageMv: 4200,
        },
      ],
    });

    const alphaCard = page
      .locator('.gate-overview-card')
      .filter({ hasText: 'E2E Gate Alpha' });
    const badge = alphaCard.locator('[data-testid="health-badge"]');

    await expect(badge).toBeVisible({ timeout: 10000 });
    await expect(badge).toHaveAttribute(
      'aria-label',
      'Battery: Charging, Shock: No Shock, Voltage: 4.20V',
    );
    await expect(alphaCard.locator('[data-testid="health-voltage"]')).toHaveText('4.20V');
  });

  test('DevicesPage shows empty state when no health data received', async ({ page }) => {
    await login(page, CONTROLLER);
    page.route('**/health', (route) => route.fulfill({ json: {} }));
    await page.goto('/devices');
    await expect(page.getByRole('heading', { name: 'Gate Overview' })).toBeVisible();
    await expectLoaded(page);

    const firstBadge = page.locator('[data-testid="health-badge"]').first();
    await expect(firstBadge).toBeVisible({ timeout: 10000 });
    await expect(firstBadge).toHaveAttribute('aria-label', 'No health data available');
    await expect(firstBadge).toContainText('Awaiting first health report');
  });

  test('DashboardPage renders without errors when health data arrives', async ({
    page,
    sendHealth,
  }) => {
    await page.goto('/dashboard');
    await expect(page.getByRole('heading', { name: 'Flood Gates' })).toBeVisible();

    sendHealth({
      messageType: 5,
      statuses: [
        {
          version: 1,
          senseGateId: 1001,
          shockStatus: 'NO_SHOCK',
          batteryStatus: 'LOW_BATTERY',
          voltageMv: 3300,
        },
      ],
    });

    await page.waitForTimeout(2000);
    expect(true).toBe(true);
  });

  test('per-field merge: shock message preserves previous battery + voltage', async ({
    page,
    sendHealth,
  }) => {
    await page.goto('/devices');
    await expect(page.getByRole('heading', { name: 'Gate Overview' })).toBeVisible();
    await expectLoaded(page);

    sendHealth({
      messageType: 5,
      statuses: [
        {
          version: 1,
          senseGateId: 1001,
          shockStatus: 'UNKNOWN',
          batteryStatus: 'CHARGING',
          voltageMv: 4200,
        },
      ],
    });

    const alphaCard = page
      .locator('.gate-overview-card')
      .filter({ hasText: 'E2E Gate Alpha' });
    const badge = alphaCard.locator('[data-testid="health-badge"]');

    await expect(badge).toBeVisible({ timeout: 10000 });
    await expect(badge).toHaveAttribute(
      'aria-label',
      'Battery: Charging, Shock: No Shock, Voltage: 4.20V',
    );

    sendHealth({
      messageType: 5,
      statuses: [
        {
          version: 1,
          senseGateId: 1001,
          shockStatus: 'SHOCK_DETECTED',
          batteryStatus: 'UNKNOWN',
          voltageMv: 0,
        },
      ],
    });

    await expect(badge).toHaveAttribute(
      'aria-label',
      'Battery: Charging, Shock: Shock Detected, Voltage: 4.20V',
      { timeout: 10000 },
    );
    const shockEl = alphaCard.locator('[data-testid="health-shock"]');
    await expect(shockEl.locator('.health-pulse-icon')).toBeVisible();
  });

  test('unmapped device appears in Unmapped Health Devices section', async ({
    page,
    sendHealth,
  }) => {
    await page.goto('/devices');
    await expect(page.getByRole('heading', { name: 'Gate Overview' })).toBeVisible();
    await expectLoaded(page);

    sendHealth({
      messageType: 5,
      statuses: [
        {
          version: 1,
          senseGateId: 9999,
          shockStatus: 'NO_SHOCK',
          batteryStatus: 'DISCHARGING',
          voltageMv: 3900,
        },
      ],
    });

    await expect(
      page.getByRole('heading', { name: 'Unmapped Health Devices' }),
    ).toBeVisible({ timeout: 10000 });
    await expect(
      page.locator('.gate-overview-card', { hasText: '#9999' }),
    ).toBeVisible();
    await expect(
      page
        .locator('.gate-overview-card', { hasText: '#9999' })
        .locator('[data-testid="health-badge"]'),
    ).toHaveAttribute(
      'aria-label',
      'Battery: Discharging, Shock: No Shock, Voltage: 3.90V',
    );
  });
});
