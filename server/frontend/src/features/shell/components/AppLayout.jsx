import { useState, useRef, useEffect } from 'react';
import { useLocation } from 'react-router-dom';
import Sidebar from './Sidebar';
import Topbar from './Topbar';
import NotificationPopup from '../../notifications/components/NotificationPopup';
import UplinkToast from '../../../shared/components/UplinkToast';
import { useAppSelector } from '../../../app/store';
import { useGetNotificationsByWorkerIdQuery } from '../../../app/store/api/api';

// Map routes to page titles
const routeTitles = {
  '/dashboard': 'Dashboard',
  '/map': 'Map',
  '/diagnostics': 'Diagnostics',
  '/devices': 'Devices',
  '/automation': 'Gate Control',
  '/logs': 'Activity Logs',
  '/settings': 'Settings',
};

export default function AppLayout({ children }) {
  const [collapsed, setCollapsed] = useState(() => {
    return localStorage.getItem('sensemante-sidebar') === 'collapsed';
  });
  const [mobileOpen, setMobileOpen] = useState(false);
  const [notificationsOpen, setNotificationsOpen] = useState(false);
  const location = useLocation();
  const title = routeTitles[location.pathname] || 'SenseMate';

  // Fetch unread notification count for badge
  const userDetails = useAppSelector((state) => state.auth.user);
  const workerId = userDetails?.workerId ?? null;
  const { data: notificationsData } = useGetNotificationsByWorkerIdQuery(workerId, { skip: !workerId });
  const unreadCount = (notificationsData ?? []).filter(n => !n.read).length;

  // Close notifications on outside click
  const notifRef = useRef(null);
  useEffect(() => {
    if (!notificationsOpen) return;
    const handler = (e) => {
      if (notifRef.current && !notifRef.current.contains(e.target)) {
        // Also ignore clicks on the notification toggle button itself
        const btn = document.getElementById('notificationButton');
        if (btn && btn.contains(e.target)) return;
        setNotificationsOpen(false);
      }
    };
    document.addEventListener('mousedown', handler);
    return () => document.removeEventListener('mousedown', handler);
  }, [notificationsOpen]);

  const handleCollapseToggle = () => {
    setCollapsed(prev => {
      const next = !prev;
      localStorage.setItem('sensemante-sidebar', next ? 'collapsed' : 'expanded');
      return next;
    });
  };

  const handleHamburgerClick = () => {
    setMobileOpen(prev => !prev);
  };

  const handleCloseMobile = () => {
    setMobileOpen(false);
  };

  const handleToggleNotifications = () => {
    setNotificationsOpen(prev => !prev);
  };

  return (
    <div className="app">
      <UplinkToast />
      <div
        className={`mobile-overlay${mobileOpen ? ' visible' : ''}`}
        onClick={handleCloseMobile}
        role="button"
        tabIndex={0}
        onKeyDown={(e) => {
          if (e.key === 'Enter' || e.key === ' ') {
            e.preventDefault();
            handleCloseMobile();
          }
        }}
        aria-label="Close menu"
      />
      <Sidebar
        collapsed={collapsed}
        mobileOpen={mobileOpen}
        onCloseMobile={handleCloseMobile}
      />
      <div className="main-area" style={{ position: 'relative' }}>
        <Topbar
          title={title}
          sidebarCollapsed={collapsed}
          onCollapseToggle={handleCollapseToggle}
          onHamburgerClick={handleHamburgerClick}
          onToggleNotifications={handleToggleNotifications}
          notificationsOpen={notificationsOpen}
          unreadCount={unreadCount}
        />
        {notificationsOpen && (
          <div ref={notifRef}>
            <NotificationPopup />
          </div>
        )}
        <main className="page-content">
          {children}
        </main>
      </div>
    </div>
  );
}
