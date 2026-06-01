import { test, expect } from '@playwright/test';
import { BACKEND_URL, CONTROLLER, VIEWER, SEEDED_GATES, apiToken } from './utils';

/**
 * Backend contract tests.
 *
 * These talk to the dockerised backend directly (root paths, no /api prefix —
 * the /api prefix only exists in the nginx/vite proxy in front of the SPA).
 * They lock down the API shape the frontend depends on, so a backend refactor
 * that breaks the contract fails here.
 */
test.describe('Backend API', () => {
  test('login returns a token and sets cookies for seeded accounts', async ({ request }) => {
    for (const account of [CONTROLLER, VIEWER]) {
      const response = await request.post(`${BACKEND_URL}/auth/login`, { data: account });
      expect(response.status()).toBe(200);
      expect((await response.json()).token).toBeTruthy();
      // Cookie-based auth: login must set HttpOnly jwt + XSRF-TOKEN
      const setCookie = response.headers()['set-cookie'];
      expect(setCookie).toContain('jwt=');
      expect(setCookie).toContain('XSRF-TOKEN=');
    }
  });

  test('login rejects invalid credentials with 401 and a message', async ({ request }) => {
    const response = await request.post(`${BACKEND_URL}/auth/login`, {
      data: { email: CONTROLLER.email, password: 'wrong-password' },
    });
    expect(response.status()).toBe(401);
    expect((await response.json()).error).toBe('Invalid email or password');
  });

  test('user-details reflects the seeded controller', async ({ request }) => {
    const { jwt } = await apiToken(request, CONTROLLER);
    const response = await request.get(`${BACKEND_URL}/auth/user-details`, {
      headers: { Authorization: `Bearer ${jwt}` },
    });

    expect(response.status()).toBe(200);
    expect(await response.json()).toMatchObject({
      email: CONTROLLER.email,
      role: 'controller',
      workerId: 1,
    });
  });

  test('gates returns the four seeded gates', async ({ request }) => {
    const response = await request.get(`${BACKEND_URL}/gates`);
    expect(response.status()).toBe(200);

    const gates = await response.json();
    expect(gates).toHaveLength(SEEDED_GATES.length);

    // Match on id so the assertion is order-independent.
    for (const expected of SEEDED_GATES) {
      const gate = gates.find((g: { id: number }) => g.id === expected.id);
      expect(gate, `gate ${expected.id} present`).toBeTruthy();
      expect(gate).toMatchObject({ location: expected.location, status: expected.status });
    }
  });

  test('notifications require auth and expose the seeded entries', async ({ request }) => {
    const unauthorized = await request.get(`${BACKEND_URL}/notifications`);
    expect(unauthorized.status()).toBe(401);

    const { jwt } = await apiToken(request, CONTROLLER);
    const response = await request.get(`${BACKEND_URL}/notifications`, {
      headers: { Authorization: `Bearer ${jwt}` },
    });
    expect(response.status()).toBe(200);

    const notifications = await response.json();
    expect(notifications).toHaveLength(2);
    expect(notifications.every((n: { read: boolean }) => n.read === false)).toBeTruthy();
  });
});
