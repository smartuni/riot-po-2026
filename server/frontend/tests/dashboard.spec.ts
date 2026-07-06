import { test, expect, expectLoaded } from './fixtures';
import { BACKEND_URL, CONTROLLER, SEEDED_GATES, apiToken, login, statusToLabel } from './utils';

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
  test.afterEach(async () => {
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }
    const gates = await (await requestContext.get(`${BACKEND_URL}/gates`)).json();
    for (const gate of gates.filter((g: { location?: string }) => g.location?.startsWith('E2E Temp'))) {
      await requestContext.delete(`${BACKEND_URL}/gates/${gate.id}`, { headers });
    }
  });

  test('renders every seeded gate with its id and status', async ({ page }) => {
    for (const gate of SEEDED_GATES) {
      const row = page.locator('table.gate-table tbody tr', { hasText: gate.location });
      await expect(row).toContainText(String(gate.id));
      // UI renders human-readable label (e.g. "Open") not DB enum ("OPEN")
      await expect(row).toContainText(statusToLabel(gate.status));
    }
  });

  test('total-gates info box matches the number of rendered rows', async ({ page }) => {
    // Compare against the live row count rather than a hardcoded number so the
    // assertion stays valid even if the mutating create-gate test is mid-run.
    const rowCount = await page.locator('table.gate-table tbody tr').count();
    expect(rowCount).toBeGreaterThanOrEqual(SEEDED_GATES.length);

    const totalBox = page.locator('.stat-card').filter({ hasText: 'Total Gates' });
    await expect(totalBox.locator('.stat-number')).toHaveText(String(rowCount));
  });

  test('search filters the gate table by location', async ({ page }) => {
    await page.getByPlaceholder('Search gates…').fill('Alpha');

    await expect(page.getByText('E2E Gate Alpha')).toBeVisible();
    await expect(page.getByText('E2E Gate Beta')).toHaveCount(0);
    await expect(page.locator('table.gate-table tbody tr')).toHaveCount(1);
  });

  test('map page renders a marker for every seeded gate', async ({ page }) => {
    // The map is now a separate /map route (MapPage.jsx), not a dashboard tab.
    await page.goto('/map');
    await expect(page.locator('.leaflet-container')).toBeVisible();

    // Markers are rendered from gate data (not network tiles), so this verifies
    // the map actually plotted the gates — and works offline / in CI where
    // OpenStreetMap tiles never load.
    await expect(page.locator('.leaflet-marker-icon')).toHaveCount(SEEDED_GATES.length);
  });

  test('expanding a gate row reveals its seeded activity', async ({ page }) => {
    const alphaRow = page.locator('table.gate-table tbody tr', { hasText: 'E2E Gate Alpha' });
    await alphaRow.getByRole('button', { name: 'expand row' }).click();

    // The seeded message also shows in the RecentActivity panel, so scope to the
    // expanded row to confirm expansion actually rendered the gate's activity.
    const expandedRow = page.locator('table.gate-table tr.expanded-row');
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
    const newRow = page.locator('table.gate-table tbody tr', { hasText: location });
    await expect(newRow).toBeVisible();
  });

  test('logout leaves the dashboard and renders the landing page', async ({ page }) => {
    // The sidebar Logout button is visible on any authenticated route.
    await page.goto('/dashboard');
    await expect(page.getByRole('heading', { name: 'Flood Gates' })).toBeVisible();

    await page.getByRole('button', { name: 'Logout' }).click();

    await expect(page).toHaveURL(/\/$/);
    // Assert the landing page content actually rendered — a URL change alone
    // would pass even if the dashboard stayed mounted with a stale session.
    await expect(page.getByText('Our Mission')).toBeVisible();
    await expect(page.getByRole('button', { name: 'Logout' })).toHaveCount(0);
  });
});

test.describe('Stat card filtering', () => {
  test.beforeEach(async ({ page }) => {
    await login(page, CONTROLLER);
    await expect(page.getByRole('heading', { name: 'Flood Gates' })).toBeVisible();
    await expectLoaded(page);
  });

  test('clicking the Open stat card filters the table to open gates', async ({ page }) => {
    const openCard = page.locator('.stat-card').filter({ hasText: 'Open' });
    await openCard.click();

    // Only OPEN gates (Alpha 1001, Gamma 1003) should be visible
    await expect(page.getByText('E2E Gate Alpha')).toBeVisible();
    await expect(page.getByText('E2E Gate Gamma')).toBeVisible();
    await expect(page.getByText('E2E Gate Beta')).toHaveCount(0);
    await expect(page.getByText('E2E Gate Delta')).toHaveCount(0);
    await expect(page.locator('table.gate-table tbody tr')).toHaveCount(2);
  });

  test('clicking an active stat card again clears the filter', async ({ page }) => {
    const openCard = page.locator('.stat-card').filter({ hasText: 'Open' });
    await openCard.click();
    await expect(page.locator('table.gate-table tbody tr')).toHaveCount(2);

    // Click again to toggle off the filter
    await openCard.click();
    await expect(page.locator('table.gate-table tbody tr')).toHaveCount(SEEDED_GATES.length);
  });

  test('clicking Total Gates shows all gates', async ({ page }) => {
    const totalCard = page.locator('.stat-card').filter({ hasText: 'Total Gates' });
    await totalCard.click();

    await expect(page.locator('table.gate-table tbody tr')).toHaveCount(SEEDED_GATES.length);
  });

  test('clicking Closed filters to closed gates', async ({ page }) => {
    const closedCard = page.locator('.stat-card').filter({ hasText: 'Closed' });
    await closedCard.click();

    // Only CLOSED gate (Beta 1002) should be visible
    await expect(page.getByText('E2E Gate Beta')).toBeVisible();
    await expect(page.locator('table.gate-table tbody tr')).toHaveCount(1);
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
