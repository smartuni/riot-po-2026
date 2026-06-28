import { useState, useRef, useEffect } from 'react';
import AppLayout from '../features/shell/components/AppLayout';
import { useAppSelector, useAppDispatch } from '../app/store';
import { setDarkMode } from '../app/store/slices/uiSlice';
import {
  useUpdateUserDetailsMutation,
} from '../app/store/api/api';

const SettingsPage = () => {
  const user = useAppSelector((state) => state.auth.user);
  const isLoading = useAppSelector((state) => state.auth.status === 'loading');
  const darkMode = useAppSelector((state) => state.ui.darkMode);
  const dispatch = useAppDispatch();
  const [updateUserDetails] = useUpdateUserDetailsMutation();

  // ── Password state ─────────────────────────────────────────
  const [currentPassword, setCurrentPassword] = useState('');
  const [newPassword, setNewPassword] = useState('');
  const [confirmPassword, setConfirmPassword] = useState('');
  const [saveStatus, setSaveStatus] = useState('idle'); // 'idle' | 'saving' | 'success' | 'error'
  const [errorMessage, setErrorMessage] = useState('');
  const saveTimerRef = useRef(null);

  useEffect(() => {
    return () => {
      if (saveTimerRef.current) clearTimeout(saveTimerRef.current);
    };
  }, []);

  const handleDarkToggle = (e) => {
    dispatch(setDarkMode(e.target.checked));
  };

  const handleSave = async () => {
    setSaveStatus('saving');

    if (!currentPassword) {
      setErrorMessage('Current password is required');
      setSaveStatus('error');
      if (saveTimerRef.current) clearTimeout(saveTimerRef.current);
      saveTimerRef.current = setTimeout(() => setSaveStatus('idle'), 5000);
      return;
    }

    if (newPassword !== confirmPassword) {
      setErrorMessage('Passwords do not match');
      setSaveStatus('error');
      if (saveTimerRef.current) clearTimeout(saveTimerRef.current);
      saveTimerRef.current = setTimeout(() => setSaveStatus('idle'), 5000);
      return;
    }

    if (newPassword.length < 6) {
      setErrorMessage('New password must be at least 6 characters');
      setSaveStatus('error');
      if (saveTimerRef.current) clearTimeout(saveTimerRef.current);
      saveTimerRef.current = setTimeout(() => setSaveStatus('idle'), 5000);
      return;
    }

    const payload = { password: currentPassword, newPassword };

    try {
      await updateUserDetails(payload).unwrap();
      setSaveStatus('success');
      setCurrentPassword('');
      setNewPassword('');
      setConfirmPassword('');
      if (saveTimerRef.current) clearTimeout(saveTimerRef.current);
      saveTimerRef.current = setTimeout(() => setSaveStatus('idle'), 3000);
    } catch {
      setErrorMessage('Save failed. Please try again.');
      setSaveStatus('error');
      if (saveTimerRef.current) clearTimeout(saveTimerRef.current);
      saveTimerRef.current = setTimeout(() => setSaveStatus('idle'), 5000);
    }
  };

  if (isLoading) {
    return (
      <AppLayout>
        <div className="page-content-inner" style={{ maxWidth: 720, margin: '0 auto', textAlign: 'center', padding: 40 }}>
          Loading...
        </div>
      </AppLayout>
    );
  }

  if (!user) {
    return (
      <AppLayout>
        <div className="page-content-inner" style={{ maxWidth: 420, margin: '0 auto', textAlign: 'center', padding: 60 }}>
          <div style={{ fontSize: 48, marginBottom: 16 }}>🔒</div>
          <h2 style={{ marginBottom: 8 }}>Sign in required</h2>
          <p style={{ color: 'var(--text-secondary)', marginBottom: 24 }}>You need to be signed in to view your settings.</p>
          <a href="/login" className="btn btn-primary" style={{ display: 'inline-block' }}>Sign In</a>
        </div>
      </AppLayout>
    );
  }

  const initials = (user.name || 'U')
    .split(' ')
    .map((p) => p[0])
    .join('')
    .toUpperCase()
    .slice(0, 2);

  return (
    <AppLayout>
        <div className="page-content-inner" style={{ maxWidth: 720, margin: '0 auto' }}>
          {/* ── Profile ─────────────────────────────────────────── */}
          <div className="settings-section">
            <h3>Profile</h3>
            <div className="profile-row">
              <div
                className="settings-avatar"
                style={{
                  width: 56,
                  height: 56,
                  borderRadius: '50%',
                  backgroundColor: 'var(--blue-600)',
                  color: '#fff',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  fontWeight: 700,
                  fontSize: 20,
                  flexShrink: 0,
                }}
              >
                {initials}
              </div>
              <div className="profile-info">
                <h4 style={{ fontSize: 18, fontWeight: 600, margin: 0 }}>
                  {user.name}
                </h4>
                <span className="role-badge">{user.role || 'Controller'}</span>
              </div>
            </div>
            <div className="form-group">
              <label className="form-label" htmlFor="settings-email">Email</label>
              <input
                id="settings-email"
                className="form-input"
                type="email"
                value={user.email || ''}
                disabled
              />
            </div>
          </div>

          {/* ── Change Password ─────────────────────────────────── */}
          <div className="settings-section">
            <h3>Change Password</h3>
            <div className="form-group">
              <label className="form-label" htmlFor="settings-current-password">Current Password</label>
              <input
                id="settings-current-password"
                className="form-input"
                type="password"
                placeholder="••••••••"
                value={currentPassword}
                onChange={(e) => setCurrentPassword(e.target.value)}
              />
            </div>
            <div className="form-group">
              <label className="form-label" htmlFor="settings-new-password">New Password</label>
              <input
                id="settings-new-password"
                className="form-input"
                type="password"
                placeholder="••••••••"
                value={newPassword}
                onChange={(e) => setNewPassword(e.target.value)}
              />
            </div>
            <div className="form-group">
              <label className="form-label" htmlFor="settings-confirm-password">Confirm New Password</label>
              <input
                id="settings-confirm-password"
                className="form-input"
                type="password"
                placeholder="••••••••"
                value={confirmPassword}
                onChange={(e) => setConfirmPassword(e.target.value)}
              />
            </div>
          </div>

          {/* ── Appearance ──────────────────────────────────────── */}
          <div className="settings-section">
            <h3>Appearance</h3>
            <div className="setting-row">
              <div>
                <div className="setting-label" style={{ fontSize: 14 }}>
                  Dark Mode
                </div>
                <div
                  className="setting-desc"
                  style={{ fontSize: 12, color: 'var(--text-secondary, #6b7280)' }}
                >
                  Switch between light and dark theme
                </div>
              </div>
              <label className="toggle-switch" id="settings-dark-toggle">
                <input
                  type="checkbox"
                  checked={darkMode}
                  onChange={handleDarkToggle}
                />
                <span className="toggle-slider" />
              </label>
            </div>
          </div>

          {/* ── Save ────────────────────────────────────────────── */}
          <div style={{ textAlign: 'right' }}>
            {saveStatus === 'success' && (
              <div style={{ color: 'var(--green-600)', marginBottom: 8 }}>Saved!</div>
            )}
            {saveStatus === 'error' && (
              <div style={{ color: 'var(--red-600)', marginBottom: 8 }}>{errorMessage}</div>
            )}
            <button
              className="btn btn-primary"
              style={{ padding: '10px 24px' }}
              onClick={handleSave}
              disabled={
                !currentPassword ||
                !newPassword ||
                !confirmPassword ||
                newPassword !== confirmPassword ||
                saveStatus === 'saving'
              }
            >
              {saveStatus === 'saving' ? 'Saving...' : 'Save Changes'}
            </button>
          </div>
        </div>
      </AppLayout>
  );
};

export default SettingsPage;
