import { test, expect, expectLoaded } from './fixtures';
import { CONTROLLER, VIEWER, login } from './utils';

test.describe('Authentication', () => {
  test('should enable submit only for valid credentials', async ({ page }) => {
    await page.goto('/login');
    await expect(page).toHaveURL(/.*login/);

    const submitButton = page.locator('button[type="submit"]');
    await expect(submitButton).toBeDisabled();

    await page.locator('input[name="email"]').fill('invalid-email');
    await expect(submitButton).toBeDisabled();

    await page.locator('input[name="email"]').fill('test@example.com');
    await page.locator('input[name="password"]').fill('short');
    await expect(submitButton).toBeDisabled();

    await page.locator('input[name="password"]').fill('password123');
    await expect(submitButton).toBeEnabled();
  });

  test('controller is routed to a fully loaded controller dashboard', async ({ page }) => {
    await login(page, CONTROLLER);
    await expect(page).toHaveURL(/\/dashboard$/);
    await expect(page.getByRole('heading', { name: 'Flood Gates' })).toBeVisible();
    await expectLoaded(page);
  });

  test('viewer is routed to a fully loaded read-only dashboard', async ({ page }) => {
    await login(page, VIEWER);
    await expect(page).toHaveURL(/\/dashboard$/);
    // Seeded data must actually render — not just the URL change.
    await expect(page.getByText('E2E Gate Alpha')).toBeVisible();
    await expectLoaded(page);
  });

  test('wrong password shows a clear error message and stays on /login', async ({ page }) => {
    await page.goto('/login');
    await page.locator('input[name="email"]').fill(CONTROLLER.email);
    await page.locator('input[name="password"]').fill('wrong-password');
    await page.locator('button[type="submit"]').click();

    // Backend returns 401 with a friendly message; the dialog must surface it
    // rather than a generic "unexpected error".
    await expect(page.getByText('Login Error')).toBeVisible();
    await expect(page.getByText('Invalid email or password')).toBeVisible();
    await expect(page).toHaveURL(/\/login$/);
  });

  test('page refresh with valid HttpOnly cookie restores session (auto-login)', async ({ page }) => {
    // Log in first to establish the cookie
    await login(page, CONTROLLER);
    await expect(page).toHaveURL(/\/dashboard$/);
    await expectLoaded(page);

    // Reload the page — the HttpOnly jwt cookie should still be present,
    // initializeAuth() should pick it up, and the user stays authenticated.
    await page.reload();
    await expect(page).toHaveURL(/\/dashboard$/);
    await expect(page.getByRole('heading', { name: 'Flood Gates' })).toBeVisible();
    await expectLoaded(page);
  });
});
