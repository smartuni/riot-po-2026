import { test, expect } from '@playwright/test';
import { CONTROLLER, SEEDED_GATES, login } from './utils';

/**
 * Frontend functionality, driven through the UI against the dockerised backend
 * with the deterministic seed. These exercise the controller dashboard's
 * interactive features (search, view toggle, expandable rows, dialogs,
 * notifications). They are intentionally non-mutating so the seeded DB stays
 * deterministic and the suite can be re-run without a reset.
 */
test.describe('Controller dashboard', () => {
  test.beforeEach(async ({ page }) => {
    await login(page, CONTROLLER);
    await expect(page.getByRole('heading', { name: 'Flood Gates' })).toBeVisible();
  });

  test('renders every seeded gate with its id and status', async ({ page }) => {
    for (const gate of SEEDED_GATES) {
      const row = page.locator('table.status-table tbody tr', { hasText: gate.location });
      await expect(row).toContainText(String(gate.id));
      await expect(row).toContainText(gate.status);
    }
  });

  test('info boxes total the seeded gate count', async ({ page }) => {
    const totalBox = page.locator('.info-boxes .box').filter({ hasText: 'Total Gate' });
    await expect(totalBox.locator('h1')).toHaveText(String(SEEDED_GATES.length));
  });

  test('search filters the gate table by location', async ({ page }) => {
    await page.getByPlaceholder('Search gates...').fill('Alpha');

    await expect(page.getByText('E2E Gate Alpha')).toBeVisible();
    await expect(page.getByText('E2E Gate Beta')).toHaveCount(0);
    await expect(page.locator('table.status-table tbody tr')).toHaveCount(1);
  });

  test('switching to map view renders the leaflet map', async ({ page }) => {
    await expect(page.locator('table.status-table')).toBeVisible();

    await page.getByRole('tab', { name: 'Map View' }).click();
    await expect(page.locator('.leaflet-container')).toBeVisible();
    await expect(page.locator('table.status-table')).toHaveCount(0);

    await page.getByRole('tab', { name: 'List View' }).click();
    await expect(page.locator('table.status-table')).toBeVisible();
  });

  test('expanding a gate row reveals its seeded activity', async ({ page }) => {
    const alphaRow = page.locator('table.status-table tbody tr', { hasText: 'E2E Gate Alpha' });
    await alphaRow.getByRole('button', { name: 'expand row' }).click();

    // The seeded message also shows in the RecentActivity panel, so scope to the
    // expanded row to confirm expansion actually rendered the gate's activity.
    const expandedRow = page.locator('table.status-table tr.expanded-row');
    await expect(expandedRow).toContainText('E2E seed: Gate 1001 OPEN');
  });

  test('notification popup shows the seeded message for the logged-in worker', async ({ page }) => {
    await page.locator('#notificationButton').click();

    await expect(page.getByText('Notifications', { exact: true })).toBeVisible();
    // Worker 1 == the logged-in controller (workerId 1); this message is seeded for them.
    await expect(page.getByText('Worker 1 should verify Gate 1001')).toBeVisible();
  });

  test('create-gate dialog stays disabled until the form is valid', async ({ page }) => {
    await page.getByRole('button', { name: 'Create Gate' }).click();

    const dialog = page.getByRole('dialog');
    await expect(dialog.getByText('Create New Gate')).toBeVisible();

    const createButton = dialog.getByRole('button', { name: 'Create', exact: true });
    await expect(createButton).toBeDisabled();

    await dialog.getByLabel('Location').fill('Test Harbor Gate');
    await dialog.getByLabel('Latitude').fill('53.55');
    await dialog.getByLabel('Longitude').fill('9.99');
    await expect(createButton).toBeEnabled();

    // Cancel instead of submitting to keep the seeded data unchanged.
    await dialog.getByRole('button', { name: 'Cancel' }).click();
    await expect(dialog).toHaveCount(0);
  });

  test('logout returns to the landing page', async ({ page }) => {
    await page.goto('/userpage');
    await page.getByRole('button', { name: 'Logout' }).click();
    await expect(page).toHaveURL(/\/$/);
  });
});

test.describe('Guest dashboard', () => {
  test('shows seeded gates without controller-only actions', async ({ page }) => {
    await page.goto('/dashboard-guest');

    await expect(page.getByText('E2E Gate Alpha')).toBeVisible();
    // The guest uses the read-only StatusTablesView; no gate-creation control.
    await expect(page.getByRole('button', { name: 'Create Gate' })).toHaveCount(0);
  });
});
