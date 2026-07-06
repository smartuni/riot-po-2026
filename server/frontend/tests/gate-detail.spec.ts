import { test, expect, expectLoaded } from './fixtures';
import { BACKEND_URL, CONTROLLER, VIEWER, SEEDED_GATES, apiToken, login } from './utils';

/**
 * Gate Detail Page (/gates/:id) — E2E tests.
 *
 * Read-only tests run in parallel against the deterministic seed (gate 1001).
 * Mutation tests run serially with afterEach cleanup to keep the seed stable.
 */

/* ── Read-only tests (parallel) ───────────────────────────────────── */

test.describe('Gate detail page (read-only)', () => {
  test.beforeEach(async ({ page }) => {
    await login(page, CONTROLLER);
    await page.goto('/gates/1001');
    await expectLoaded(page);
  });

  test('displays gate header with id, location, and status badge', async ({ page }) => {
    await expect(page.getByText('G-1001')).toBeVisible();
    await expect(page.getByRole('heading', { name: 'E2E Gate Alpha' })).toBeVisible();
    await expect(page.locator('.status-badge').filter({ hasText: 'Open' })).toBeVisible();
  });

  test('shows gate information card with all fields', async ({ page }) => {
    const infoCard = page.locator('.card').filter({ hasText: 'Gate Information' });
    await expect(infoCard).toBeVisible();

    await expect(infoCard).toContainText('Device ID');
    await expect(infoCard).toContainText('501');

    await expect(infoCard).toContainText('Confidence');
    await expect(infoCard).toContainText('90%');

    await expect(infoCard).toContainText('Priority');
    await expect(infoCard).toContainText('3');

    await expect(infoCard).toContainText('Height above NN');
    await expect(infoCard).toContainText('2.5 m');

    await expect(infoCard).toContainText('State Confirmation');

    await expect(infoCard).toContainText('Coordinates');
    await expect(infoCard).toContainText('53.55000, 9.99370');
  });

  test('activities card shows seeded activities', async ({ page }) => {
    const activitiesCard = page.locator('.card .card-header').filter({ hasText: 'Activities' }).locator('..');
    await expect(activitiesCard).toBeVisible();

    await expect(activitiesCard.locator('.activity-list')).toBeVisible();
    await expect(activitiesCard.locator('.activity-list')).toContainText('E2E seed: Gate 1001 OPEN');
  });

  test('back button navigates to dashboard', async ({ page }) => {
    await page.getByRole('button', { name: 'Back to Dashboard' }).click();

    await expect(page).toHaveURL(/\/dashboard$/);
    await expect(page.getByRole('heading', { name: 'Flood Gates' })).toBeVisible();
  });

  test('controller sees action buttons', async ({ page }) => {
    await expect(page.getByRole('button', { name: 'Set Status Manually' })).toBeVisible();
    await expect(page.getByRole('button', { name: 'Request Change' })).toBeVisible();
    await expect(page.getByRole('button', { name: 'Delete Gate' })).toBeVisible();
  });

  test('viewer does not see action buttons or edit controls', async ({ page }) => {
    // beforeEach logged in as CONTROLLER — must logout before logging in as VIEWER
    await page.getByRole('button', { name: 'Logout' }).click();
    await expect(page).toHaveURL(/\/$/);
    await login(page, VIEWER);
    await page.goto('/gates/1001');
    await expectLoaded(page);

    // No controller-only action buttons
    await expect(page.getByRole('button', { name: 'Set Status Manually' })).toHaveCount(0);
    await expect(page.getByRole('button', { name: 'Request Change' })).toHaveCount(0);
    await expect(page.getByRole('button', { name: 'Delete Gate' })).toHaveCount(0);

    // No priority MUI Select dropdown
    await expect(page.locator('.MuiSelect-select')).toHaveCount(0);

    // No height edit pencil button (action-link with EditIcon svg)
    await expect(page.locator('button.action-link').filter({ has: page.locator('svg') })).toHaveCount(0);
  });
});

/* ── Mutation tests (serial) ─────────────────────────────────────── */

test.describe('Gate detail page (mutations)', () => {
  test.describe.configure({ mode: 'serial' });

  test.beforeEach(async ({ page }) => {
    await login(page, CONTROLLER);
  });

  // ── g) editing height above NN ──

  test('editing height above NN updates the displayed value', async ({ page }) => {
    await page.goto('/gates/1001');
    await expectLoaded(page);

    // Scope to the Gate Information card to find the height edit pencil button
    const infoCard = page.locator('.card').filter({ hasText: 'Gate Information' });
    const heightSection = infoCard.locator('div', { hasText: 'Height above NN' }).first();
    const editPencil = heightSection.locator('button.action-link').filter({ has: page.locator('svg') });

    await editPencil.click();

    // An input appears — fill with 7.7
    const heightInput = infoCard.locator('input.form-input[type="number"]');
    await heightInput.fill('7.7');

    // Click the check button (action-link with CheckIcon svg)
    const checkButton = heightSection.locator('button.action-link').filter({ has: page.locator('svg') }).nth(0);
    await checkButton.click();

    // Assert the height now displays 7.7 m
    await expect(infoCard).toContainText('7.7 m');
  });

  test.afterEach(async () => {
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }
    await requestContext.put(`${BACKEND_URL}/update-height/1001`, {
      headers,
      data: { heightAboveNN: 2.5 },
    });
  });

  // ── h) editing priority ──

  test('editing priority updates the displayed value', async ({ page }) => {
    await page.goto('/gates/1001');
    await expectLoaded(page);

    // Scope to the Gate Information card for the MUI Select
    const infoCard = page.locator('.card').filter({ hasText: 'Gate Information' });
    const prioritySelect = infoCard.locator('.MuiSelect-select');

    await prioritySelect.click();

    // Click the MenuItem with text "1" from the dropdown
    await page.getByRole('option', { name: '1', exact: true }).click();

    // Assert priority displays "1"
    await expect(infoCard).toContainText('Priority');
    const prioritySection = infoCard.locator('div', { hasText: 'Priority' }).first();
    await expect(prioritySection).toContainText('1');
  });

  test.afterEach(async () => {
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }
    await requestContext.put(`${BACKEND_URL}/update-priority/1001`, {
      headers,
      data: { priority: 3 },
    });
  });

  // ── i) manual status override ──

  test('manual status override changes status and shows manual override banner', async ({ page }) => {
    // Create a temp gate via API
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }

    await requestContext.post(`${BACKEND_URL}/add-gate-ui`, {
      headers,
      data: {
        location: 'E2E Temp Manual Gate',
        latitude: 53.5,
        longitude: 10.0,
        priority: 0,
        status: 'OPEN',
      },
    });

    // GET /gates to find the temp gate ID
    const gates = await (await requestContext.get(`${BACKEND_URL}/gates`)).json();
    const tempGate = gates.find((g: { location: string }) => g.location === 'E2E Temp Manual Gate');
    const tempGateId = tempGate.id;

    await page.goto(`/gates/${tempGateId}`);
    await expectLoaded(page);

    // Click "Set Status Manually" button
    await page.getByRole('button', { name: 'Set Status Manually' }).click();

    // Assert dialog title visible
    const dialog = page.getByRole('dialog');
    await expect(dialog.getByText('Set Gate Status Manually')).toBeVisible();

    // Click the "Closed" selector button (inside the dialog)
    await dialog.getByRole('button', { name: 'Closed' }).click();

    // Click "Set Status" submit button
    await dialog.getByRole('button', { name: 'Set Status' }).click();

    // Assert dialog closes
    await expect(dialog).toHaveCount(0);

    // Assert status badge shows "Closed"
    await expect(page.locator('.status-badge').filter({ hasText: 'Closed' })).toBeVisible();

    // Assert manual override banner visible
    await expect(page.getByText('Status was set manually by an operator')).toBeVisible();
  });

  test.afterEach(async () => {
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }
    // Clean up any temp manual gates
    const gates = await (await requestContext.get(`${BACKEND_URL}/gates`)).json();
    for (const gate of gates.filter((g: { location?: string }) => g.location?.startsWith('E2E Temp Manual Gate'))) {
      await requestContext.delete(`${BACKEND_URL}/gates/${gate.id}`, { headers });
    }
  });

  // ── j) deleting a gate navigates back to dashboard ──

  test('deleting a gate navigates back to dashboard', async ({ page }) => {
    // Create a temp gate via API
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }

    await requestContext.post(`${BACKEND_URL}/add-gate-ui`, {
      headers,
      data: {
        location: 'E2E Temp Delete Gate',
        latitude: 53.5,
        longitude: 10.0,
        priority: 0,
        status: 'OPEN',
      },
    });

    // GET /gates to find the temp gate ID
    const gates = await (await requestContext.get(`${BACKEND_URL}/gates`)).json();
    const tempGate = gates.find((g: { location: string }) => g.location === 'E2E Temp Delete Gate');
    const tempGateId = tempGate.id;

    await page.goto(`/gates/${tempGateId}`);
    await expectLoaded(page);

    // Click "Delete Gate" button
    await page.getByRole('button', { name: 'Delete Gate' }).click();

    // Assert confirm dialog visible
    const dialog = page.getByRole('dialog');
    await expect(dialog.getByText('Confirm Deletion')).toBeVisible();

    // Click "Delete" button inside the dialog
    await dialog.getByRole('button', { name: 'Delete' }).click();

    // Assert URL is /dashboard
    await expect(page).toHaveURL(/\/dashboard$/);

    // Assert "Flood Gates" heading visible
    await expect(page.getByRole('heading', { name: 'Flood Gates' })).toBeVisible();
  });

  test.afterEach(async () => {
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }
    // Best-effort DELETE of any temp delete gates that might remain
    const gates = await (await requestContext.get(`${BACKEND_URL}/gates`)).json();
    for (const gate of gates.filter((g: { location?: string }) => g.location?.startsWith('E2E Temp Delete Gate'))) {
      await requestContext.delete(`${BACKEND_URL}/gates/${gate.id}`, { headers });
    }
  });
});
