import { test, expect, request as apiRequest } from '@playwright/test';
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
  test('login sets HttpOnly jwt and XSRF-TOKEN cookies for seeded accounts', async () => {
    for (const account of [CONTROLLER, VIEWER]) {
      // Use a fresh request context per account so cookie state is independent.
      const ctx = await apiRequest.newContext();
      const response = await ctx.post(`${BACKEND_URL}/auth/login`, { data: account });
      expect(response.status()).toBe(200);
      // Cookie-based auth: login must set HttpOnly jwt + XSRF-TOKEN cookies.
      const setCookies = response.headersArray()
        .filter(h => h.name.toLowerCase() === 'set-cookie')
        .map(h => h.value);
      expect(setCookies.some(c => c.includes('jwt='))).toBeTruthy();
      expect(setCookies.some(c => c.includes('XSRF-TOKEN='))).toBeTruthy();
      await ctx.dispose();
    }
  });

  test('login rejects invalid credentials with 401 and a message', async ({ request }) => {
    const response = await request.post(`${BACKEND_URL}/auth/login`, {
      data: { email: CONTROLLER.email, password: 'wrong-password' },
    });
    expect(response.status()).toBe(401);
    expect((await response.json()).error).toBe('Invalid email or password');
  });

  test('user-details reflects the seeded controller', async () => {
    const { requestContext } = await apiToken(CONTROLLER);
    const response = await requestContext.get(`${BACKEND_URL}/auth/user-details`);

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

    const { requestContext } = await apiToken(CONTROLLER);
    const response = await requestContext.get(`${BACKEND_URL}/notifications`);
    expect(response.status()).toBe(200);

    const notifications = await response.json();
    expect(notifications).toHaveLength(2);
    expect(notifications.every((n: { read: boolean }) => n.read === false)).toBeTruthy();
  });

  test('update-height changes the heightAboveNN for a gate', async () => {
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }

    const update = await requestContext.put(`${BACKEND_URL}/update-height/1001`, {
      data: { heightAboveNN: 9.99 },
      headers,
    });
    expect([200, 204]).toContain(update.status());

    const gates = await (await requestContext.get(`${BACKEND_URL}/gates`)).json();
    const gate1001 = gates.find((g: { id: number }) => g.id === 1001);
    expect(gate1001, 'gate 1001 present').toBeTruthy();
    expect(gate1001.heightAboveNN).toBe(9.99);

    // CLEANUP: restore the seeded height.
    await requestContext.put(`${BACKEND_URL}/update-height/1001`, {
      data: { heightAboveNN: 2.5 },
      headers,
    });
  });

  test('set-status overrides gate status and sets manualOverride', async () => {
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }

    // Create a temp gate — manualOverride can't be reset via API, so don't
    // mutate a seeded gate. add-gate-ui returns plain text, not JSON.
    const create = await requestContext.post(`${BACKEND_URL}/add-gate-ui`, {
      data: { location: 'E2E Temp Status Gate', latitude: 53.5, longitude: 10.0, priority: 0, status: 'OPEN' },
      headers,
    });
    expect(create.status()).toBe(200);

    const gates = await (await requestContext.get(`${BACKEND_URL}/gates`)).json();
    const tempGate = gates.find((g: { location: string }) => g.location === 'E2E Temp Status Gate');
    expect(tempGate, 'temp gate created').toBeTruthy();
    const tempGateId = tempGate.id;

    const setStatus = await requestContext.post(`${BACKEND_URL}/gates/${tempGateId}/1/set-status`, {
      data: { status: 'CLOSED' },
      headers,
    });
    expect(setStatus.status()).toBe(200);

    const updated = await (await requestContext.get(`${BACKEND_URL}/gates`)).json();
    const gate = updated.find((g: { id: number }) => g.id === tempGateId);
    expect(gate, 'temp gate still present after set-status').toBeTruthy();
    expect(gate.status).toBe('CLOSED');
    expect(gate.manualOverride).toBe(true);

    // CLEANUP: remove the temp gate.
    await requestContext.delete(`${BACKEND_URL}/gates/${tempGateId}`, { headers });
  });
});
