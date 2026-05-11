import { render, screen } from '@testing-library/react';
import App from './App';
import { test, expect } from 'vitest';

test('renders Sense Mate description', () => {
  render(<App />);
  const descriptionElement = screen.getByText(/Sense Mate is an intelligent dashboard/i);
  expect(descriptionElement).toBeInTheDocument();
});