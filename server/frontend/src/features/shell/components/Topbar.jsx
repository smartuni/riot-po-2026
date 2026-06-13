import { useState, useRef, useEffect } from 'react';
import { useAppSelector } from '../../../app/store';
import LogoutButton from '../../auth/components/LogoutButton';
import DarkModeToggle from '../../../shared/components/DarkModeToggle';
import CollapseToggle from '../../../shared/components/CollapseToggle';

export default function Topbar({
  title,
  sidebarCollapsed,
  onCollapseToggle,
  onHamburgerClick,
  onToggleNotifications,
  notificationsOpen,
  unreadCount = 0,
}) {
  const user = useAppSelector((state) => state.auth.user);
  const [menuOpen, setMenuOpen] = useState(false);
  const menuRef = useRef(null);

  const initials = user
    ? user.name?.split(' ').map(n => n[0]).join('').toUpperCase() || 'U'
    : '…';

  // Close menu on outside click
  useEffect(() => {
    if (!menuOpen) return;
    const handler = (e) => {
      if (menuRef.current && !menuRef.current.contains(e.target)) setMenuOpen(false);
    };
    document.addEventListener('mousedown', handler);
    return () => document.removeEventListener('mousedown', handler);
  }, [menuOpen]);

  return (
    <header className="topbar">
      <div className="topbar-left">
        <button className="hamburger" onClick={onHamburgerClick} type="button">☰</button>
        <CollapseToggle collapsed={sidebarCollapsed} onToggle={onCollapseToggle} />
        <span className="topbar-title">{title}</span>
      </div>
      <div className="topbar-right">
        <button
          id="notificationButton"
          className={`topbar-btn${notificationsOpen ? ' active' : ''}`}
          title="Notifications"
          type="button"
          onClick={onToggleNotifications}
        >
          🔔{unreadCount > 0 && <span className="notif-badge">{unreadCount}</span>}
        </button>
        <DarkModeToggle />
        <div className="topbar-avatar-wrapper" ref={menuRef}>
          <button
            className="topbar-avatar"
            type="button"
            onClick={() => setMenuOpen(prev => !prev)}
            aria-label="User menu"
          >
            {initials}
          </button>
          {menuOpen && (
            <div className="topbar-avatar-menu">
              <div className="topbar-avatar-menu-header">
                <strong>{user ? user.name || 'User' : 'Loading…'}</strong>
                {user?.email && <span className="topbar-avatar-menu-email">{user.email}</span>}
              </div>
              <div className="topbar-avatar-menu-divider" />
              <div className="topbar-avatar-menu-logout">
                <LogoutButton />
              </div>
            </div>
          )}
        </div>
      </div>
    </header>
  );
}
