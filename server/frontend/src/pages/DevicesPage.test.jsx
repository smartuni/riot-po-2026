import { render, screen } from '@testing-library/react';
import { Provider } from 'react-redux';
import { BrowserRouter } from 'react-router-dom';
import { test, expect } from 'vitest';
import { store } from '../app/store';
import DevicesPage from './DevicesPage';

test('renders Devices page without crashing', () => {
  render(
    <Provider store={store}>
      <BrowserRouter>
        <DevicesPage />
      </BrowserRouter>
    </Provider>
  );
  const main = screen.getByRole('main');
  expect(main).toBeInTheDocument();
});
