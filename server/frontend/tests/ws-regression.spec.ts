import { test, expect } from './fixtures';
import { BACKEND_URL, CONTROLLER, login } from './utils';

test.describe('WebSocket regression — stale lastTimeStamp bug', () => {
  test.beforeEach(async ({ page }) => {
    await login(page, CONTROLLER);
    await expect(page.getByRole('heading', { name: 'Flood Gates' })).toBeVisible();
  });

  test('gate status change via device uplink updates lastTimeStamp in UI', async ({
    page,
    request,
  }) => {
    // 1. Read initial state via API
    const initialGates = await (await request.get(`${BACKEND_URL}/gates`)).json();
    const alphaBefore = initialGates.find((g: { id: number }) => g.id === 1001);
    expect(alphaBefore.status).toBe('OPEN');

    // 2. Trigger uplink: status 1 = CLOSED
    const triggerTime = Math.floor(Date.now() / 1000);
    const response = await request.post(`${BACKEND_URL}/e2e/simulate-uplink`, {
      data: {
        deviceName: 'sensegate-1',
        messageType: 1,
        statuses: [{ gateId: 1001, status: 1, timestamp: triggerTime }],
      },
    });
    expect(response.status()).toBe(200);

    // 3. Wait for Gate 1001 status to change to CLOSED in the UI (WS pushes update)
    const row = page.locator('table.gate-table tbody tr', {
      hasText: 'E2E Gate Alpha',
    });
    await expect(row).toContainText(/closed/i, { timeout: 5000 });

    // 4. KEY BUG ASSERTION: "Last Update" must show a fresh relative time.
    //    Before fix: WS sends stale lastTimeStamp → UI shows "5 months ago" (from Jan 2026 seed) → test FAILS.
    //    After fix:  WS sends fresh lastTimeStamp → UI shows "seconds ago"                     → test PASSES.
    const lastUpdateCell = row.locator('td[data-label="Last Update"]');
    await expect(lastUpdateCell).toContainText(/seconds? ago|minute/, { timeout: 5000 });

    // 5. Verify via API that DB has a fresh timestamp
    const gatesAfter = await (await request.get(`${BACKEND_URL}/gates`)).json();
    const alphaAfter = gatesAfter.find((g: { id: number }) => g.id === 1001);
    expect(new Date(alphaAfter.lastTimeStamp).getTime()).toBeGreaterThan(
      new Date(alphaBefore.lastTimeStamp).getTime(),
    );

    // 6. Verify an activity entry was added for gate 1001
    const activities = await (
      await request.get(`${BACKEND_URL}/gate-activities`)
    ).json();
    const gateActivities = activities.filter(
      (a: { gateId: number }) => a.gateId === 1001,
    );
    expect(gateActivities.length).toBeGreaterThanOrEqual(2); // 1 seed + 1 new
  });

  test('state confirmation change updates lastTimeStamp', async ({
    page,
    request,
  }) => {
    // 1. Read initial state for gate 1002 (Beta)
    const gatesBefore = await (await request.get(`${BACKEND_URL}/gates`)).json();
    const betaBefore = gatesBefore.find((g: { id: number }) => g.id === 1002);

    // 2. Trigger state confirmation change
    const response = await request.post(
      `${BACKEND_URL}/e2e/simulate-state-confirmation`,
      {
        data: { gateId: 1002, state: 'WORKER_CONFIRMED_SINGLE' },
      },
    );
    expect(response.status()).toBe(200);

    // 3. Verify gate 1002 state confirmation badge updated in UI
    const row = page.locator('table.gate-table tbody tr', {
      hasText: 'E2E Gate Beta',
    });
    // WORKER_CONFIRMED_SINGLE renders a ✓ icon via CheckIcon.
    // The Badge badgeContent contains the icon; the cell should still show CLOSED status.
    const statusCell = row.locator('td[data-label="Status"]');
    await expect(statusCell).toContainText(/closed/i, { timeout: 5000 });

    // 4. Verify via API that lastTimeStamp is no longer the stale seed value
    const gatesAfter = await (await request.get(`${BACKEND_URL}/gates`)).json();
    const betaAfter = gatesAfter.find((g: { id: number }) => g.id === 1002);
    expect(new Date(betaAfter.lastTimeStamp).getTime()).toBeGreaterThan(
      new Date(betaBefore.lastTimeStamp).getTime(),
    );
  });

  test.afterEach(async ({ request }) => {
    // Reset gate 1001 back to OPEN (status 0) to keep the suite idempotent.
    try {
      await request.post(`${BACKEND_URL}/e2e/simulate-uplink`, {
        data: {
          deviceName: 'sensegate-1',
          messageType: 1,
          statuses: [
            { gateId: 1001, status: 0, timestamp: Math.floor(Date.now() / 1000) },
          ],
        },
      });
    } catch {
      // Best-effort reset — don't fail the suite if reset fails.
    }

    // Reset gate 1002 state confirmation back to CONFIRMED
    try {
      await request.post(`${BACKEND_URL}/e2e/simulate-state-confirmation`, {
        data: { gateId: 1002, state: 'CONFIRMED' },
      });
    } catch {
      // Best-effort
    }
  });
});
