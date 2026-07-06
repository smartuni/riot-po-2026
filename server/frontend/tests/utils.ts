import { expect, request, type Page, type APIRequestContext } from '@playwright/test';

/**
 * Shared fixtures for the E2E suite.
 *
 * These tests run against the dockerised backend (profile `e2e`) with the
 * deterministic seed from `server/backend/.../resources/data-e2e.sql` (H2
 * in-memory database). Bring it up with `server/backend/scripts/e2e-reset.sh`
 * before running.
 *
 * The constants below mirror that seed and the test accounts in
 * `application.yml`. If the seed changes, update them here in one place.
 */

/** Backend container, reachable directly for API-contract tests (no /api prefix). */
export const BACKEND_URL = 'http://localhost:8080';

export const CONTROLLER = { email: 'test@example.com', password: 'test123' };
export const VIEWER = { email: 'test2@example.com', password: 'test234' };

/** The four gates seeded by data-e2e.sql, in id order. */
export const SEEDED_GATES = [
  { id: 1001, location: 'E2E Gate Alpha', status: 'OPEN',
    stateConfirmation: 'WORKER_CONFIRMED_SINGLE', heightAboveNN: 2.5,
    priority: 3, confidence: 90, deviceId: 501, manualOverride: false },
  { id: 1002, location: 'E2E Gate Beta', status: 'CLOSED',
    stateConfirmation: 'WORKER_CONFIRMED_MULTI', heightAboveNN: 3.8,
    priority: 2, confidence: 85, deviceId: 502, manualOverride: false },
  { id: 1003, location: 'E2E Gate Gamma', status: 'OPEN',
    stateConfirmation: 'UNCONFIRMED', heightAboveNN: 1.2,
    priority: 1, confidence: 70, deviceId: 503, manualOverride: false },
  { id: 1004, location: 'E2E Gate Delta', status: 'OUT_OF_SERVICE',
    stateConfirmation: 'WORKER_CONFLICT', heightAboveNN: 5.0,
    priority: 0, confidence: 40, deviceId: 504, manualOverride: false },
];

/**
 * Log in through the UI and wait for the post-login redirect.
 * All authenticated users land on /dashboard; role-based rendering is handled in-page.
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

  await expect(page).toHaveURL(/\/dashboard$/);
}

/**
 * Map DB status enum to the human-readable label rendered by the UI.
 * StatusTables.jsx transforms "OPEN" → "Open", "CLOSED" → "Closed", etc.
 */
export function statusToLabel(status: string): string {
  switch (status) {
    case 'OPEN':
      return 'Open';
    case 'CLOSED':
      return 'Closed';
    case 'OUT_OF_SERVICE':
      return 'Out of Service';
    default:
      return status;
  }
}

/** Extract XSRF-TOKEN from all Set-Cookie headers in a response. */
function extractXSRFToken(response: import('@playwright/test').APIResponse): string | null {
  const setCookies = response.headersArray()
    .filter(h => h.name.toLowerCase() === 'set-cookie')
    .map(h => h.value);
  for (const cookie of setCookies) {
    const match = cookie.match(/XSRF-TOKEN=([^;]+)/);
    if (match) return match[1];
  }
  return null;
}

/** POST /auth/login against the backend. Returns authenticated context + CSRF token. */
export async function apiToken(
  credentials: { email: string; password: string },
): Promise<{ requestContext: APIRequestContext; csrfToken: string | null }> {
  const requestContext = await request.newContext();
  const response = await requestContext.post(`${BACKEND_URL}/auth/login`, { data: credentials });
  return {
    requestContext,
    csrfToken: extractXSRFToken(response),
  };
}
