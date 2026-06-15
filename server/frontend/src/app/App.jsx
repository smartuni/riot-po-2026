import '../shared/styles/App.css';
import { useEffect } from 'react';
import { Provider } from 'react-redux';
import { BrowserRouter, Routes, Route } from 'react-router-dom';
import { store, useAppDispatch } from './store';
import { initializeAuth } from './store/slices/authSlice';
import LandingPage from '../pages/LandingPage';
import LoginPage from '../pages/LoginPage';
import DashboardPage from '../pages/DashboardPage';
import RegisterPage from '../pages/RegisterPage';
import UserPage from '../pages/UserPage';
import DashboardViewPage from '../pages/DashboardViewPage';
import DashboardGuestPage from '../pages/DashboardGuestPage';
import { ProtectedRoute, PublicOnlyRoute } from '../features/auth';

function AppContent() {
  const dispatch = useAppDispatch();

  useEffect(() => {
    dispatch(initializeAuth());
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

        {/* Protected — redirect to /login if not authenticated */}
        <Route element={<ProtectedRoute />}>
          <Route path="/userpage" element={<UserPage />} />
        </Route>

        {/* Protected + role-gated */}
        <Route element={<ProtectedRoute roles={['controller']} />}>
          <Route path="/dashboard" element={<DashboardPage />} />
        </Route>
        <Route element={<ProtectedRoute roles={['controller', 'viewer']} />}>
          <Route path="/dashboard-view" element={<DashboardViewPage />} />
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
