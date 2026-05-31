import { expect, type Page } from '@playwright/test';

/**
 * Shared fixtures for the E2E suite.
 *
 * These tests run against the dockerised backend (profile `e2e`) with the
 * deterministic seed from `server/backend/.../db/migration-e2e/V100__e2e_seed.sql`.
 * Bring it up with `server/backend/scripts/e2e-reset.sh` before running.
 *
 * The constants below mirror that seed and the test accounts in
 * `application.yml`. If the seed changes, update them here in one place.
 */

/** Backend container, reachable directly for API-contract tests (no /api prefix). */
export const BACKEND_URL = 'http://localhost:8080';

export const CONTROLLER = { email: 'test@example.com', password: 'test123' };
export const VIEWER = { email: 'test2@example.com', password: 'test234' };

/** The four gates seeded by V100, in id order. */
export const SEEDED_GATES = [
  { id: 1001, location: 'E2E Gate Alpha', status: 'OPEN' },
  { id: 1002, location: 'E2E Gate Beta', status: 'CLOSED' },
  { id: 1003, location: 'E2E Gate Gamma', status: 'OPEN' },
  { id: 1004, location: 'E2E Gate Delta', status: 'OUT_OF_SERVICE' },
];

/**
 * Log in through the UI and wait for the post-login redirect.
 * Controllers land on /dashboard, everyone else on /dashboard-view.
 */
export async function login(
  page: Page,
  credentials: { email: string; password: string },
): Promise<void> {
  await page.goto('/login');
  await page.locator('input[name="email"]').fill(credentials.email);
  await page.locator('input[name="password"]').fill(credentials.password);

  const submit = page.locator('button[type="submit"]');
  await expect(submit).toBeEnabled();
  await submit.click();

  await expect(page).toHaveURL(/\/dashboard(-view)?$/);
}

/** POST /auth/login against the backend and return the JWT. */
export async function apiToken(
  request: { post: (url: string, opts: object) => Promise<{ json: () => Promise<{ token: string }> }> },
  credentials: { email: string; password: string },
): Promise<string> {
  const response = await request.post(`${BACKEND_URL}/auth/login`, { data: credentials });
  return (await response.json()).token;
}
