import '../shared/styles/App.css';
import { BrowserRouter, Routes, Route } from 'react-router-dom';
import { AuthProvider } from '../features/auth';
import LandingPage from '../pages/LandingPage';
import LoginPage from '../pages/LoginPage';
import DashboardPage from '../pages/DashboardPage';
import RegisterPage from '../pages/RegisterPage';
import UserPage from '../pages/UserPage';
import DashboardViewPage from '../pages/DashboardViewPage';
import DashboardGuestPage from '../pages/DashboardGuestPage';

function App() {
  return (
    <AuthProvider>
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
    </AuthProvider>
  );
}

export default App;
