import { NavLink, Link } from 'react-router-dom';
import { useAppSelector } from '../../../app/store';
import LogoutButton from '../../auth/components/LogoutButton';
export default function Sidebar({ collapsed, mobileOpen, onCloseMobile }) {
  const closeMobile = () => {
    if (onCloseMobile) onCloseMobile();
  };

  const user = useAppSelector((state) => state.auth.user);
  const initials = user
    ? user.name?.split(' ').map(n => n[0]).join('').toUpperCase() || 'U'
    : '…';
  const displayName = user ? user.name || 'User' : 'Loading…';
  const role = user?.role ? (user.role.charAt(0).toUpperCase() + user.role.slice(1)) : '';

  return (
    <aside className={`sidebar${collapsed ? ' collapsed' : ''}${mobileOpen ? ' mobile-open' : ''}`}>
      {/* Logo */}
      <div className="sidebar-logo">
        <div className="logo-icon">SM</div>
        <span className="logo-text">SenseMate</span>
      </div>

      {/* Navigation */}
      <nav className="sidebar-nav">
        <NavLink to="/dashboard" end className={({ isActive }) => `nav-item${isActive ? ' active' : ''}`} onClick={closeMobile}>
          <span className="nav-icon">📊</span><span className="nav-label">Dashboard</span>
        </NavLink>
        <NavLink to="/map" className={({ isActive }) => `nav-item${isActive ? ' active' : ''}`} onClick={closeMobile}>
          <span className="nav-icon">🗺️</span><span className="nav-label">Map</span>
        </NavLink>
        <NavLink to="/diagnostics" className={({ isActive }) => `nav-item${isActive ? ' active' : ''}`} onClick={closeMobile}>
          <span className="nav-icon">🔍</span><span className="nav-label">Diagnostics</span>
          <span className="soon-badge">Soon</span>
        </NavLink>
        <NavLink to="/devices" className={({ isActive }) => `nav-item${isActive ? ' active' : ''}`} onClick={closeMobile}>
          <span className="nav-icon">📱</span><span className="nav-label">Devices</span>
        </NavLink>
        <NavLink to="/automation" className={({ isActive }) => `nav-item${isActive ? ' active' : ''}`} onClick={closeMobile}>
          <span className="nav-icon">⚡</span><span className="nav-label">Gate Control</span>
          <span className="soon-badge">Soon</span>
        </NavLink>
        <NavLink to="/logs" className={({ isActive }) => `nav-item${isActive ? ' active' : ''}`} onClick={closeMobile}>
          <span className="nav-icon">📋</span><span className="nav-label">Logs</span>
        </NavLink>
        <NavLink to="/settings" className={({ isActive }) => `nav-item${isActive ? ' active' : ''}`} onClick={closeMobile}>
          <span className="nav-icon">⚙️</span><span className="nav-label">Settings</span>
        </NavLink>
      </nav>

      {/* User Section */}
      <div className="sidebar-user">
        <div className="user-avatar">{initials}</div>
        <div className="user-info">
          <div className="user-name">{displayName}</div>
          <span className="user-role">{role}</span>
        </div>
        <div className="sidebar-logout">
          <LogoutButton />
        </div>
      </div>
    </aside>
  );
}
