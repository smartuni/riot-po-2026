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
    const infoPanel = page.locator('.detail-tab-panel').first();
    await expect(infoPanel).toBeVisible();

    await expect(infoPanel).toContainText('Device ID');
    await expect(infoPanel).toContainText('501');

    await expect(infoPanel).toContainText('Confidence');
    await expect(infoPanel).toContainText('90%');

    await expect(infoPanel).toContainText('Priority');
    await expect(infoPanel).toContainText('3');

    await expect(infoPanel).toContainText('Height above NN');
    await expect(infoPanel).toContainText('2.5 m');

    await expect(infoPanel).toContainText('State Confirmation');

    await expect(infoPanel).toContainText('Coordinates');
    await expect(infoPanel).toContainText('53.55000, 9.99370');
  });

  test('activities tab shows seeded activities', async ({ page }) => {
    await page.locator('.detail-tab').nth(2).click();
    await expectLoaded(page);

    const activitiesPanel = page.locator('.detail-tab-panel').first();
    await expect(activitiesPanel).toBeVisible();

    await expect(activitiesPanel.locator('.activity-list')).toBeVisible();
    await expect(activitiesPanel.locator('.activity-list')).toContainText('E2E seed: Gate 1001 OPEN');
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

  test('tabs are visible and switchable', async ({ page }) => {
    const tabs = page.locator('.detail-tab');
    await expect(tabs).toHaveCount(3);

    await expect(tabs.nth(0)).toContainText('Overview');
    await expect(tabs.nth(1)).toContainText('Metadata');
    await expect(tabs.nth(2)).toContainText('Activities');

    await tabs.nth(1).click();
    await expect(page.locator('.detail-tab-panel')).toContainText('Closing Threshold');

    await tabs.nth(2).click();
    await expect(page.locator('.detail-tab-panel')).toContainText('E2E seed');

    await tabs.nth(0).click();
    await expect(page.locator('.detail-tab-panel')).toContainText('Device ID');
  });

  test('viewer does not see action buttons or edit controls', async ({ page }) => {
    await page.getByRole('button', { name: 'Logout' }).click();
    await expect(page).toHaveURL(/\/$/);
    await login(page, VIEWER);
    await page.goto('/gates/1001');
    await expectLoaded(page);

    await expect(page.getByRole('button', { name: 'Set Status Manually' })).toHaveCount(0);
    await expect(page.getByRole('button', { name: 'Request Change' })).toHaveCount(0);
    await expect(page.getByRole('button', { name: 'Delete Gate' })).toHaveCount(0);

    await expect(page.locator('.MuiSelect-select')).toHaveCount(0);

    await page.locator('.detail-tab').nth(1).click();
    await page.waitForTimeout(500);
    await expect(page.getByRole('button', { name: 'Add' })).toHaveCount(0);
  });
});

/* ── Mutation tests (serial) ─────────────────────────────────────── */

test.describe('Gate detail page (mutations)', () => {
  test.describe.configure({ mode: 'serial' });

  test.beforeEach(async ({ page }) => {
    await login(page, CONTROLLER);
  });

  // Consolidated cleanup: runs once after each test. Handles all mutation
  // cleanup in the correct order so one failing step doesn't block the others.
  test.afterEach(async () => {
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }

    // 1. Clean up test_key metadata on gate 1001
    try {
      const metaResp = await requestContext.get(`${BACKEND_URL}/gates/1001/metadata`);
      if (metaResp.ok()) {
        const metadata = await metaResp.json();
        for (const m of metadata.filter((m: { key: string; id: number }) => m.key === 'test_key')) {
          await requestContext.delete(`${BACKEND_URL}/gates/1001/metadata/${m.id}`, { headers });
        }
      }
    } catch (e) {
      console.warn('Metadata cleanup failed:', e);
    }

    // 2. Reset height above NN for gate 1001
    try {
      await requestContext.put(`${BACKEND_URL}/update-height/1001`, {
        headers,
        data: { heightAboveNN: 2.5 },
      });
    } catch (e) {
      console.warn('Height reset failed:', e);
    }

    // 3. Reset priority for gate 1001
    try {
      await requestContext.put(`${BACKEND_URL}/update-priority/1001`, {
        headers,
        data: { priority: 3 },
      });
    } catch (e) {
      console.warn('Priority reset failed:', e);
    }

    // 4. Clean up temp gates (both "E2E Temp Manual Gate" and "E2E Temp Delete Gate")
    try {
      const gatesResp = await requestContext.get(`${BACKEND_URL}/gates`);
      if (gatesResp.ok()) {
        const gates = await gatesResp.json();
        for (const gate of gates.filter((g: { location?: string }) =>
          g.location?.startsWith('E2E Temp Manual Gate') || g.location?.startsWith('E2E Temp Delete Gate'))) {
          await requestContext.delete(`${BACKEND_URL}/gates/${gate.id}`, { headers });
        }
      }
    } catch (e) {
      console.warn('Temp gate cleanup failed:', e);
    }
  });

  // ── g) metadata card displays seeded metadata and allows CRUD ──

  test('metadata card displays seeded metadata and allows CRUD', async ({ page }) => {
    await page.goto('/gates/1001');
    await expectLoaded(page);

    await page.locator('.detail-tab').nth(1).click();
    await page.waitForTimeout(500);

    const metadataPanel = page.locator('.detail-tab-panel').first();

    await expect(metadataPanel).toContainText('Closing Threshold');
    await expect(metadataPanel).toContainText('Height above NN');
    await expect(metadataPanel).toContainText('2.5 m');

    await expect(metadataPanel).toContainText('Additional Metadata');
    await expect(metadataPanel).toContainText('closing_threshold_cm');
    await expect(metadataPanel).toContainText('max_water_flow');

    await page.getByRole('button', { name: 'Add' }).click();

    const keyInput = metadataPanel.locator('input').nth(0);
    const valueInput = metadataPanel.locator('input').nth(1);
    await keyInput.fill('test_key');
    await valueInput.fill('test_value');

    const checkButton = metadataPanel.getByRole('button', { name: 'Add metadata item' });
    await checkButton.click();

    await expect(metadataPanel).toContainText('test_key');
    await expect(metadataPanel).toContainText('test_value');

    const testRow = metadataPanel.locator('div', { hasText: 'test_key' }).first();
    const editPencil = testRow.locator('button.action-link').filter({ has: page.locator('svg') }).first();
    await editPencil.click();

    const editValueInput = metadataPanel.locator('input').nth(1);
    await editValueInput.fill('updated_value');

    const saveCheck = metadataPanel.getByRole('button', { name: 'Save metadata edit' });
    await saveCheck.click();

    await expect(metadataPanel).toContainText('updated_value');

    const deleteRow = metadataPanel.locator('div', { hasText: 'test_key' }).first();
    const deleteButton = deleteRow.getByRole('button', { name: 'Delete metadata item' });
    await deleteButton.click();

    await expect(metadataPanel).not.toContainText('test_key');
  });

  // ── g2) editing height above NN updates the displayed value ──

  test('editing height above NN updates the displayed value', async ({ page }) => {
    await page.goto('/gates/1001');
    await expectLoaded(page);

    await page.locator('.detail-tab').nth(1).click();
    await page.waitForTimeout(500);

    const metadataPanel = page.locator('.detail-tab-panel').first();
    const editPencil = metadataPanel.locator('button.action-link').filter({ has: page.locator('svg') }).first();

    await editPencil.click();

    const heightInput = metadataPanel.locator('input[type="number"]');
    await heightInput.fill('7.7');

    await heightInput.press('Enter');

    await expect(metadataPanel).toContainText('7.7 m');
  });

  // ── h) editing priority ──

  test('editing priority updates the displayed value', async ({ page }) => {
    await page.goto('/gates/1001');
    await expectLoaded(page);

    const overviewPanel = page.locator('.detail-tab-panel').first();
    const prioritySelect = overviewPanel.locator('.MuiSelect-select');

    await prioritySelect.click();

    await page.getByRole('option', { name: '1', exact: true }).click();

    await expect(overviewPanel).toContainText('Priority');
    const prioritySection = overviewPanel.locator('div', { hasText: 'Priority' }).first();
    await expect(prioritySection).toContainText('1');
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
});
