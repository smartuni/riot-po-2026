import logo from './logo.svg';
import './styles/App.css';
import { BrowserRouter, Routes, Route } from 'react-router-dom';
import LandingPage from './components/LandingPage'
import LoginPage from './components/LoginPage';
import Dashboard from './components/Dashboard';
import GatesPage from './components/GatesPage';
import RegisterPage from './components/RegisterPage';
import MapPage from './components/MapPage';
import EventLogs from "./components/EventLogs";
import UserPage from './components/UserPage';
import DashboardView from './components/DashboardView';

function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<LandingPage />}></Route>
        <Route path="/login" element={<LoginPage />}></Route>
        <Route path="/register" element={<RegisterPage />}></Route>
        {/*<Route path='/gates' element={<GatesPage/>}></Route>*/}
        <Route path='/dashboard' element={<Dashboard />}></Route>
        <Route path='/map-view' element={<MapPage />}></Route>
        <Route path='/event-logs' element={<EventLogs />}></Route>
        <Route path='/userpage' element={<UserPage />}></Route>
        <Route path='/dashboard-view' element={<DashboardView />}></Route>
      </Routes>
    </BrowserRouter>
  );
}

export default App;

