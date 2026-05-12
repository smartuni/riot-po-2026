import '../shared/styles/App.css';
import { useEffect } from 'react';
import { useDispatch, Provider } from 'react-redux';
import { BrowserRouter, Routes, Route } from 'react-router-dom';
import { store } from './store';
import { APP_START, APP_STOP } from './store/middleware/wsMiddleware';
import LandingPage from '../pages/LandingPage';
import LoginPage from '../pages/LoginPage';
import DashboardPage from '../pages/DashboardPage';
import RegisterPage from '../pages/RegisterPage';
import UserPage from '../pages/UserPage';
import DashboardViewPage from '../pages/DashboardViewPage';
import DashboardGuestPage from '../pages/DashboardGuestPage';

function AppContent() {
  const dispatch = useDispatch();

  useEffect(() => {
    dispatch({ type: APP_START });
    return () => {
      dispatch({ type: APP_STOP });
    };
  }, [dispatch]);

  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<LandingPage />}></Route>
        <Route path="/login" element={<LoginPage />}></Route>
        <Route path="/register" element={<RegisterPage />}></Route>
        <Route path='/dashboard' element={<DashboardPage />}></Route>
        <Route path='/userpage' element={<UserPage />}></Route>
        <Route path='/dashboard-view' element={<DashboardViewPage />}></Route>
        <Route path='/dashboard-guest' element={<DashboardGuestPage />}></Route>
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
