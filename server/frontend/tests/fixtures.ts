import { test as base, expect, type Page } from '@playwright/test';

export { expect };

/**
 * Test wrapper that fails if the page logged a console error or threw an
 * uncaught error during the test. Browser-level "Failed to load resource"
 * lines are filtered out so intentional negative-path requests (e.g. a 401)
 * don't trip it — assert on those explicitly instead.
 */
export const test = base.extend<{ consoleErrors: string[] }>({
  consoleErrors: [
    async ({ page }, use) => {
      const errors: string[] = [];
      page.on('pageerror', (e) => {
        if (!/WebSocket|ws:\/\//.test(e.message)) {
          errors.push(`pageerror: ${e.message}`);
        }
      });
      page.on('console', (m) => {
        if (m.type() === 'error' && !/Failed to load resource/.test(m.text()) && !/WebSocket|ws:\/\//.test(m.text())) {
          errors.push(m.text());
        }
      });
      await use(errors);
      expect(errors, 'no console errors during the test').toEqual([]);
    },
    { auto: true },
  ],
});

/**
 * Assert the page has finished loading: no MUI/ARIA loading spinners remain.
 * toHaveCount auto-retries, so this also waits out the transient loading state.
 */
export async function expectLoaded(page: Page): Promise<void> {
  await expect(page.locator('.MuiCircularProgress-root, [role="progressbar"]')).toHaveCount(0);
}
