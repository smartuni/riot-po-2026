import { render, screen } from '@testing-library/react';
import App from './App';
import { test, expect } from 'vitest';

test('renders SenseMate landing page', () => {
  render(<App />);
  const headingElement = screen.getByRole('heading', { name: /SenseMate/i });
  expect(headingElement).toBeInTheDocument();
  const descriptionElement = screen.getByText(/Intelligent Flood Gate Monitoring/i);
  expect(descriptionElement).toBeInTheDocument();
});