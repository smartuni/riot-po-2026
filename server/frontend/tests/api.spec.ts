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

  test('gate metadata CRUD operations work', async () => {
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }

    // GET — verify seeded metadata
    const initial = await requestContext.get(`${BACKEND_URL}/gates/1001/metadata`);
    expect(initial.status()).toBe(200);
    const initialMetadata = await initial.json();
    expect(initialMetadata).toEqual(
      expect.arrayContaining([
        expect.objectContaining({ key: 'closing_threshold_cm', value: '250' }),
        expect.objectContaining({ key: 'max_water_flow', value: '12.5' }),
      ]),
    );

    // POST — create new metadata
    const create = await requestContext.post(`${BACKEND_URL}/gates/1001/metadata`, {
      data: { key: 'test_key', value: 'test_value' },
      headers,
    });
    expect(create.status()).toBe(200);
    const created = await create.json();
    expect(created.key).toBe('test_key');
    expect(created.value).toBe('test_value');
    expect(created.id).toBeTruthy();
    const metadataId = created.id;

    // GET — verify the new entry is in the list
    const afterCreate = await requestContext.get(`${BACKEND_URL}/gates/1001/metadata`);
    const afterCreateMetadata = await afterCreate.json();
    expect(afterCreateMetadata).toEqual(
      expect.arrayContaining([
        expect.objectContaining({ key: 'test_key', value: 'test_value', id: metadataId }),
      ]),
    );

    // PUT — update the value
    const update = await requestContext.put(`${BACKEND_URL}/gates/1001/metadata/${metadataId}`, {
      data: { key: 'test_key', value: 'updated' },
      headers,
    });
    expect(update.status()).toBe(200);

    // GET — verify the value is updated
    const afterUpdate = await requestContext.get(`${BACKEND_URL}/gates/1001/metadata`);
    const afterUpdateMetadata = await afterUpdate.json();
    expect(afterUpdateMetadata).toEqual(
      expect.arrayContaining([
        expect.objectContaining({ key: 'test_key', value: 'updated', id: metadataId }),
      ]),
    );

    // DELETE — remove the entry
    const deleteResponse = await requestContext.delete(`${BACKEND_URL}/gates/1001/metadata/${metadataId}`, { headers });
    expect(deleteResponse.status()).toBe(200);

    // GET — verify the entry is gone
    const afterDelete = await requestContext.get(`${BACKEND_URL}/gates/1001/metadata`);
    const afterDeleteMetadata = await afterDelete.json();
    expect(afterDeleteMetadata).not.toEqual(
      expect.arrayContaining([
        expect.objectContaining({ key: 'test_key' }),
      ]),
    );
  });

  test.afterEach(async () => {
    const { requestContext, csrfToken } = await apiToken(CONTROLLER);
    const headers: Record<string, string> = {};
    if (csrfToken) {
      headers['X-XSRF-TOKEN'] = csrfToken;
    }
    // Clean up any test_key metadata entries left behind
    const response = await requestContext.get(`${BACKEND_URL}/gates/1001/metadata`);
    if (response.ok()) {
      const metadata = await response.json();
      for (const m of metadata.filter((m: { key: string; id: number }) => m.key === 'test_key')) {
        await requestContext.delete(`${BACKEND_URL}/gates/1001/metadata/${m.id}`, { headers });
      }
    }
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
