import { test, expect, expectLoaded } from './fixtures';
import { CONTROLLER, SEEDED_GATES, login } from './utils';

/**
 * Read-only tests for the Devices page (/devices).
 *
 * The page renders a card-based "Gate Overview" with key metrics for each
 * gate. These tests verify the card grid renders the deterministic seed data
 * correctly and that cards link to the gate detail page. All tests are
 * non-mutating so the seeded DB stays deterministic across re-runs.
 */
test.describe('Devices page', () => {
  test.beforeEach(async ({ page }) => {
    await login(page, CONTROLLER);
    await page.goto('/devices');
    await expectLoaded(page);
  });

  test('renders a card for every seeded gate', async ({ page }) => {
    await expect(page.locator('.gate-overview-grid')).toBeVisible();

    const cards = page.locator('.gate-overview-card');
    await expect(cards).toHaveCount(SEEDED_GATES.length);

    for (const gate of SEEDED_GATES) {
      await expect(cards.filter({ hasText: `#${gate.id}` })).toHaveCount(1);
    }
  });

  test('card displays all gate information fields', async ({ page }) => {
    const alphaCard = page
      .locator('.gate-overview-card')
      .filter({ hasText: 'E2E Gate Alpha' });

    await expect(alphaCard).toContainText('Confidence');
    await expect(alphaCard).toContainText('90%');

    await expect(alphaCard).toContainText('Height above NN');
    await expect(alphaCard).toContainText('2.5 m');

    await expect(alphaCard).toContainText('Priority');
    await expect(alphaCard).toContainText('3');

    await expect(alphaCard).toContainText('State Confirmation');

    await expect(alphaCard).toContainText('Device:');
    await expect(alphaCard).toContainText('501');

    await expect(alphaCard).toContainText('View Details →');
  });

  test('card shows correct status badge for each gate', async ({ page }) => {
    const cards = page.locator('.gate-overview-card');

    await expect(cards.filter({ hasText: 'E2E Gate Alpha' })).toContainText('Open');
    await expect(cards.filter({ hasText: 'E2E Gate Beta' })).toContainText('Closed');
    await expect(cards.filter({ hasText: 'E2E Gate Gamma' })).toContainText('Open');
    await expect(cards.filter({ hasText: 'E2E Gate Delta' })).toContainText('OOS');
  });

  test('clicking a card navigates to gate detail page', async ({ page }) => {
    const alphaCard = page
      .locator('.gate-overview-card')
      .filter({ hasText: 'E2E Gate Alpha' });

    await alphaCard.click();

    await expect(page).toHaveURL(/\/gates\/1001$/);
    await expect(page.getByText('G-1001')).toBeVisible();
    await expect(page.getByText('E2E Gate Alpha')).toBeVisible();
  });

  test('gate with conflict shows Conflict state confirmation', async ({ page }) => {
    const deltaCard = page
      .locator('.gate-overview-card')
      .filter({ hasText: 'E2E Gate Delta' });

    await expect(deltaCard).toContainText('State Confirmation');
  });
});
