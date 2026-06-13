import { useEffect } from 'react';
import { Provider } from 'react-redux';
import { BrowserRouter, Routes, Route } from 'react-router-dom';
import { store, useAppDispatch } from './store';
import { initializeAuth } from './store/slices/authSlice';
import { ProtectedRoute, PublicOnlyRoute } from '../features/auth';
import LandingPage from '../pages/LandingPage';
import LoginPage from '../pages/LoginPage';
import DashboardPage from '../pages/DashboardPage';
import DashboardGuestPage from '../pages/DashboardGuestPage';
import RegisterPage from '../pages/RegisterPage';
import MapPage from '../pages/MapPage';
import DiagnosticsPage from '../pages/DiagnosticsPage';
import DevicesPage from '../pages/DevicesPage';
import AutomationPage from '../pages/AutomationPage';
import LogsPage from '../pages/LogsPage';
import SettingsPage from '../pages/SettingsPage';

function AppContent() {
  const dispatch = useAppDispatch();

  useEffect(() => {
    dispatch(initializeAuth());
    // Initialize dark mode from localStorage
    const saved = localStorage.getItem('sensemante-dark');
    if (saved === 'true' || (!saved && window.matchMedia('(prefers-color-scheme: dark)').matches)) {
      document.body.classList.add('dark');
    }
  }, [dispatch]);

  return (
    <BrowserRouter>
      <Routes>
        {/* Public — no auth required */}
        <Route path="/" element={<LandingPage />} />
        <Route path="/dashboard-guest" element={<DashboardGuestPage />} />

        {/* PublicOnly — redirect to /dashboard if already logged in */}
        <Route element={<PublicOnlyRoute />}>
          <Route path="/login" element={<LoginPage />} />
          <Route path="/register" element={<RegisterPage />} />
        </Route>

        {/* Protected + role-gated */}
        <Route element={<ProtectedRoute roles={['controller', 'viewer']} />}>
          <Route path="/dashboard" element={<DashboardPage />} />
          <Route path="/map" element={<MapPage />} />
          <Route path="/diagnostics" element={<DiagnosticsPage />} />
          <Route path="/devices" element={<DevicesPage />} />
          <Route path="/automation" element={<AutomationPage />} />
          <Route path="/logs" element={<LogsPage />} />
          <Route path="/settings" element={<SettingsPage />} />
        </Route>
      </Routes>
    </BrowserRouter>
  );
}

function App() {
  return (
    <Provider store={store}>
      <AppContent />
    </Provider>
  );
}

export default App;
