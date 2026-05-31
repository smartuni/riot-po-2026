import { test, expect, expectLoaded } from './fixtures';
import { BACKEND_URL, CONTROLLER, SEEDED_GATES, apiToken, login } from './utils';

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
    // The dashboard must finish loading — no spinners left hanging.
    await expectLoaded(page);
  });

  // Remove any gate created by the mutating create-gate test so the seed stays
  // deterministic, even if that test failed before its inline cleanup.
  test.afterEach(async ({ request }) => {
    const token = await apiToken(request, CONTROLLER);
    const gates = await (await request.get(`${BACKEND_URL}/gates`)).json();
    for (const gate of gates.filter((g: { location?: string }) => g.location?.startsWith('E2E Temp'))) {
      await request.delete(`${BACKEND_URL}/gates/${gate.id}`, {
        headers: { Authorization: `Bearer ${token}` },
      });
    }
  });

  test('renders every seeded gate with its id and status', async ({ page }) => {
    for (const gate of SEEDED_GATES) {
      const row = page.locator('table.status-table tbody tr', { hasText: gate.location });
      await expect(row).toContainText(String(gate.id));
      await expect(row).toContainText(gate.status);
    }
  });

  test('total-gates info box matches the number of rendered rows', async ({ page }) => {
    // Compare against the live row count rather than a hardcoded number so the
    // assertion stays valid even if the mutating create-gate test is mid-run.
    const rowCount = await page.locator('table.status-table tbody tr').count();
    expect(rowCount).toBeGreaterThanOrEqual(SEEDED_GATES.length);

    const totalBox = page.locator('.info-boxes .box').filter({ hasText: 'Total Gate' });
    await expect(totalBox.locator('h1')).toHaveText(String(rowCount));
  });

  test('search filters the gate table by location', async ({ page }) => {
    await page.getByPlaceholder('Search gates...').fill('Alpha');

    await expect(page.getByText('E2E Gate Alpha')).toBeVisible();
    await expect(page.getByText('E2E Gate Beta')).toHaveCount(0);
    await expect(page.locator('table.status-table tbody tr')).toHaveCount(1);
  });

  test('map view renders a marker for every seeded gate', async ({ page }) => {
    await expect(page.locator('table.status-table')).toBeVisible();

    await page.getByRole('tab', { name: 'Map View' }).click();
    await expect(page.locator('.leaflet-container')).toBeVisible();
    await expect(page.locator('table.status-table')).toHaveCount(0);

    // Markers are rendered from gate data (not network tiles), so this verifies
    // the map actually plotted the gates — and works offline / in CI where
    // OpenStreetMap tiles never load.
    await expect(page.locator('.leaflet-marker-icon')).toHaveCount(SEEDED_GATES.length);

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

  test('creating a gate validates the form and adds the gate to the table', async ({ page }) => {
    // Unique "E2E Temp" prefix so afterEach can clean it up via the API.
    const location = 'E2E Temp Harbor Gate';

    await page.getByRole('button', { name: 'Create Gate' }).click();

    const dialog = page.getByRole('dialog');
    await expect(dialog.getByText('Create New Gate')).toBeVisible();

    const createButton = dialog.getByRole('button', { name: 'Create', exact: true });
    await expect(createButton).toBeDisabled();

    await dialog.getByLabel('Location').fill(location);
    await dialog.getByLabel('Latitude').fill('53.55');
    await dialog.getByLabel('Longitude').fill('9.99');
    await expect(createButton).toBeEnabled();

    await createButton.click();
    await expect(dialog).toHaveCount(0);

    // The new gate must show up in the table (RTK Query invalidates and refetches).
    const newRow = page.locator('table.status-table tbody tr', { hasText: location });
    await expect(newRow).toBeVisible();
  });

  test('logout leaves the user page and renders the landing page', async ({ page }) => {
    await page.goto('/userpage');
    await expect(page.getByRole('button', { name: 'Logout' })).toBeVisible();

    await page.getByRole('button', { name: 'Logout' }).click();

    await expect(page).toHaveURL(/\/$/);
    // Assert the landing page content actually rendered — a URL change alone
    // would pass even if the user-page stayed mounted with a stale session.
    await expect(page.getByText('Our Mission')).toBeVisible();
    await expect(page.getByRole('button', { name: 'Logout' })).toHaveCount(0);
  });
});

test.describe('Guest dashboard', () => {
  test('shows seeded gates without controller-only actions', async ({ page }) => {
    await page.goto('/dashboard-guest');

    await expect(page.getByText('E2E Gate Alpha')).toBeVisible();
    await expectLoaded(page);
    // The guest uses the read-only StatusTablesView; no gate-creation control.
    await expect(page.getByRole('button', { name: 'Create Gate' })).toHaveCount(0);
  });
});
