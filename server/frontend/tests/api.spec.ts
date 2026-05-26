import { test, expect } from '@playwright/test';

test.describe('Backend API Tests', () => {
  test('should authenticate and return token', async ({ request }) => {
    const response = await request.post('http://localhost:8080/api/auth/login', {
      data: { email: 'test@example.com', password: 'test123' }
    });
    
    expect(response.status()).toBe(200);
    const body = await response.json();
    expect(body).toHaveProperty('token');
  });

  test('should fetch gates with valid token', async ({ request }) => {
    const loginResponse = await request.post('http://localhost:8080/api/auth/login', {
      data: { email: 'test@example.com', password: 'test123' }
    });
    const token = (await loginResponse.json()).token;
    
    const gatesResponse = await request.get('http://localhost:8080/api/gates', {
      headers: { Authorization: `Bearer ${token}` }
    });
    
    expect(gatesResponse.status()).toBe(200);
    expect(Array.isArray(await gatesResponse.json())).toBeTruthy();
  });
});
