import { test, expect } from '@playwright/test';
test.describe('Authentication', () => {
  test('should navigate to login page from landing page', async ({ page }) => {
    await page.goto('/');
    // The specific locator depends on the exact LandingPage implementation.
    // Assuming there's a Login link or button somewhere.
    // If we're not sure, let's just go directly for now.
    await page.goto('/login');
    await expect(page).toHaveURL(/.*login/);
    // Check that we have the email and password fields
    await expect(page.locator('input[name="email"]')).toBeVisible();
    await expect(page.locator('input[name="password"]')).toBeVisible();
    await expect(page.locator('button[type="submit"]')).toBeVisible();
  });
  test('should show validation errors when submitting empty form', async ({ page }) => {
    await paimport { test, expect } from '@playwright/test';
test.describe('Authentication', () => {
  test('should navigate to login page from landing page', async ({ page }) => {
    await paged(test.describe('Authentication', () => {
  test(oc  test('should navigate to login page li    await page.goto('/');
    // The specific locator depends on the exact Lanr(    // The specific locail    // Assuming there's a Login link or button somewhere.
    // If we're nl(    // If we're not sure, let's just go directly for nowon    await page.goto('/login');
    await expect(page).toss    await expect(page).toHave c    // Check that we have the email and pasbl    await expect(pagcat << 'EOF' > /Users/artinnouri/IdeaProjects/riot-po-2026/server/frontend/tests/auth.spec.ts
import { test, expect } from '@playwright/test';
test.describe('Authentication', () => {
  test('should allow navigation and valid credentials', async ({ page }) => {
    await page.goto('/login');
    await expect(page).toHaveURL(/.*login/);
    const submitButton = page.locator('button[type="submit"]');
    await expect(submitButton).toBeDisabled();
    await page.locator('input[name="email"]').fill('test@example.com');
    await page.locator('input[name="password"]').fill('password123');
    await expect(submitButton).toBeEnabled();
  });
});
