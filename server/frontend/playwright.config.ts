import { defineConfig, devices } from '@playwright/test';

/**
 * Read environment variables from file.
 * https://github.com/motdotla/dotenv
 */
// import dotenv from 'dotenv';
// import path from 'path';
// dotenv.config({ path: path.resolve(__dirname, '.env') });

/**
 * See https://playwright.dev/docs/test-configuration.
 */
export default defineConfig({
  testDir: './tests',
  /* Run tests in files in parallel */
  fullyParallel: true,
  /* Fail the build on CI if you accidentally left test.only in the source code. */
  forbidOnly: !!process.env.CI,
  /* Retry on CI only */
  retries: process.env.CI ? 2 : 0,
  /* Opt out of parallel tests on CI. */
  workers: process.env.CI ? 1 : undefined,
  /* Reporter to use. See https://playwright.dev/docs/test-reporters */
  reporter: 'html',
  /* Shared settings for all the projects below. See https://playwright.dev/docs/api/class-testoptions. */
  use: {
    /* Base URL to use in actions like `await page.goto('')`. */
    baseURL: 'http://localhost:5173',

    /* Collect trace when retrying the failed test. See https://playwright.dev/docs/trace-viewer */
    trace: 'on-first-retry',
  },

  /* Configure projects for major browsers */
  projects: [
    {
      name: 'chromium',
      use: { ...devices['Desktop Chrome'] },
    },

    {
      name: 'firefox',
      use: { ...devices['Desktop Firefox'] },
    },

    {
      name: 'webkit',
      use: { ...devices['Desktop Safari'] },
    },

    /* Test against mobile viewports. */
    // {
    //   name: 'Mobile Chrome',
    //   use: { ...devices['Pixel 5'] },
    // },
    // {
    //   name: 'Mobile Safari',
    //   use: { ...devices['iPhone 12'] },
    // },

    /* Test against branded browsers. */
    // {
    //   name: 'Microsoft Edge',
    //   use: { ...devices['Desktop Edge'], channel: 'msedge' },
    // },
    // {
    //   name: 'Google Chrome',
    //   use: { ...devices['Desktop Chrome'], channel: 'chrome' },
    // },
  ],

  /* Spin up the dockerised backend + the Vite dev server before the tests.
     Playwright starts each server, waits for its url to respond, then runs. */
  webServer: [
    {
      // Bring up backend (e2e profile with H2 in-memory DB, deterministic seed).
      // --build picks up source changes. Foreground `up` lets Playwright own the
      // lifecycle and tear it down on exit. No PostgreSQL needed — H2 is embedded.
      command:
        'docker compose -f ../docker-compose.e2e.yml down && ' +
        'docker compose -f ../docker-compose.e2e.yml up --build backend',
      url: 'http://localhost:8080/actuator/health',
      // Reuse whatever is already healthy (local e2e-reset.sh or a CI-managed backend)
      // instead of tearing it down and rebuilding.
      reuseExistingServer: true,
      // Cold start = image build (no maven cache in CI) + Spring Boot + H2 init.
      timeout: 360_000,
      stdout: 'pipe',
      stderr: 'pipe',
    },
    {
      command: 'npm run start',
      url: 'http://localhost:5173',
      reuseExistingServer: !process.env.CI,
    },
  ],
});
