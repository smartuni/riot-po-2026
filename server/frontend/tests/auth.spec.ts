import { test, expect } from '@playwright/test';

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
});
