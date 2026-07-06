import { test, expect, expectLoaded } from './fixtures';
import { BACKEND_URL, CONTROLLER, VIEWER, SEEDED_GATES, apiToken, login } from './utils';

/**
 * Automation page (/automation) — water-level risk assessment, edit mode,
 * viewer read-only enforcement, and propose-changes mutation.
 *
 * Tests a–e are non-mutating and run in parallel. Test f mutates the
 * requestedStatus of seeded gates and runs serially with an afterEach that
 * resets the DB so the seed stays deterministic.
 */
test.describe('Automation page', () => {
  test.describe.configure({ mode: 'parallel' });

  test.beforeEach(async ({ page }) => {
    await login(page, CONTROLLER);
    await page.goto('/automation');
    await expectLoaded(page);
  });

  test('water level input and calculate shows risk assessment table', async ({ page }) => {
    await page.locator('#water-level-input').fill('4');
    await page.getByRole('button', { name: 'Calculate' }).click();

    // Water level 4 → Medium risk (> 3)
    await expect(page.getByText('Medium', { exact: true })).toBeVisible();

    // Gate consequence assessment table is rendered
    await expect(page.locator('.gate-table')).toBeVisible();

    // All seeded gates appear as rows in the table
    for (const gate of SEEDED_GATES) {
      await expect(
        page.locator('.gate-table tbody tr', { hasText: gate.location }),
      ).toBeVisible();
    }
  });

  test('critical water level marks all gates for close action', async ({ page }) => {
    await page.locator('#water-level-input').fill('9');
    await page.getByRole('button', { name: 'Calculate' }).click();

    // Water level 9 → Critical risk (> 8)
    await expect(page.getByText('Critical', { exact: true })).toBeVisible();

    // At critical level every non-CLOSED gate (including out-of-service) is
    // recommended for closure. CLOSED gates show "Already closed".
    for (const gate of SEEDED_GATES) {
      const row = page.locator('.gate-table tbody tr', { hasText: gate.location });
      const actionCell = row.locator('td').last();
      if (gate.status === 'CLOSED') {
        await expect(actionCell).toContainText('Already closed');
      } else {
        await expect(actionCell).toContainText('Close');
      }
    }

    // Critical warning mentions out-of-service gates
    await expect(page.getByText('all gates (including out-of-service)')).toBeVisible();
  });

  test('low water level shows open gates for closure', async ({ page }) => {
    await page.locator('#water-level-input').fill('1');
    await page.getByRole('button', { name: 'Calculate' }).click();

    // Water level 1 → Low risk (≤ 3)
    await expect(page.getByText('Low', { exact: true })).toBeVisible();

    const alphaRow = page.locator('.gate-table tbody tr', { hasText: 'E2E Gate Alpha' });
    const betaRow = page.locator('.gate-table tbody tr', { hasText: 'E2E Gate Beta' });
    const gammaRow = page.locator('.gate-table tbody tr', { hasText: 'E2E Gate Gamma' });
    const deltaRow = page.locator('.gate-table tbody tr', { hasText: 'E2E Gate Delta' });

    // Open gates are recommended for closure
    await expect(alphaRow.locator('td').last()).toContainText('Close');
    await expect(gammaRow.locator('td').last()).toContainText('Close');
    // Closed gate is already closed
    await expect(betaRow.locator('td').last()).toContainText('Already closed');
    // Out-of-service gate needs no action at low water level
    await expect(deltaRow.locator('td').last()).toContainText('No action');
  });

  test('edit mode pre-selects gates needing close', async ({ page }) => {
    await page.locator('#water-level-input').fill('4');
    await page.getByRole('button', { name: 'Calculate' }).click();

    await page.getByRole('button', { name: /Edit Mode/ }).click();

    // Checkboxes appear for every gate row
    await expect(page.locator('.gate-table input[type="checkbox"]')).toHaveCount(
      SEEDED_GATES.length,
    );

    // Alpha (OPEN, needs close) — pre-selected
    const alphaRow = page.locator('.gate-table tbody tr', { hasText: 'E2E Gate Alpha' });
    await expect(alphaRow.locator('input[type="checkbox"]')).toBeChecked();

    // Gamma (OPEN, needs close) — pre-selected
    const gammaRow = page.locator('.gate-table tbody tr', { hasText: 'E2E Gate Gamma' });
    await expect(gammaRow.locator('input[type="checkbox"]')).toBeChecked();

    // Beta (CLOSED, already closed) — NOT pre-selected
    const betaRow = page.locator('.gate-table tbody tr', { hasText: 'E2E Gate Beta' });
    await expect(betaRow.locator('input[type="checkbox"]')).not.toBeChecked();
  });

  test('viewer sees read-only alert and no edit mode toggle', async ({ page }) => {
    // beforeEach logged in as CONTROLLER — switch to VIEWER
    await page.getByRole('button', { name: 'Logout' }).click();
    await expect(page).toHaveURL(/\/$/);
    await login(page, VIEWER);
    await page.goto('/automation');
    await expectLoaded(page);

    await page.locator('#water-level-input').fill('4');
    await page.getByRole('button', { name: 'Calculate' }).click();

    // Viewer sees a read-only info alert
    await expect(page.getByText('read-only access')).toBeVisible();
    // Edit Mode toggle is controller-only
    await expect(page.getByRole('button', { name: /Edit Mode/ })).toHaveCount(0);
  });
});

test.describe('Automation page (mutations)', () => {
  test.describe.configure({ mode: 'serial' });

  test.beforeEach(async ({ page }) => {
    await login(page, CONTROLLER);
    await page.goto('/automation');
    await expectLoaded(page);
  });

  // Reset requestedStatus for all seeded gates so the DB stays deterministic
  // across re-runs. Worker 1 is the controller account.
  test.afterEach(async () => {
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }
    for (const gate of SEEDED_GATES) {
      await requestContext.post(`${BACKEND_URL}/${gate.id}/1/request-status-change/`, {
        headers,
        data: { requestedStatus: 'REQUESTED_NONE' },
      });
    }
  });

  test('propose changes sends REQUESTED_CLOSE for selected gates', async ({ page }) => {
    // Critical water level → all non-CLOSED gates need closing
    await page.locator('#water-level-input').fill('9');
    await page.getByRole('button', { name: 'Calculate' }).click();

    await page.getByRole('button', { name: /Edit Mode/ }).click();

    const proposeButton = page.getByRole('button', { name: 'Propose Changes' });
    await expect(proposeButton).toBeEnabled();
    await proposeButton.click();

    // Confirmation dialog appears
    const dialog = page.getByRole('dialog');
    await expect(dialog).toBeVisible();
    await expect(dialog.getByText('Confirm Gate Changes')).toBeVisible();

    // Confirm the changes
    await dialog.getByRole('button', { name: 'Confirm' }).click();

    // Success alert confirms the request was sent
    await expect(page.getByText('Successfully requested close')).toBeVisible();
  });
});
