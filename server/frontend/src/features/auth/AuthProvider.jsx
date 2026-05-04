import { useState, useEffect, useCallback } from 'react';
import apiClient from '../../shared/api/apiClient';
import { getCookie, setCookie, eraseCookie } from '../../shared/utils/cookie';
import {
  loadUserDetails,
  loginUser,
  registerUser,
  logoutUser,
  updateUserDetails,
} from './api/authApi';
import AuthContext from './AuthContext';

export function AuthProvider({ children }) {
  const [user, setUser] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const isAuthenticated = !!user;

  // On mount, try to restore session from existing JWT cookie
  useEffect(() => {
    const jwt = getCookie('jwt');
    if (jwt) {
      apiClient.defaults.headers.common['Authorization'] = `Bearer ${jwt}`;
      loadUserDetails()
        .then((data) => setUser(data))
        .catch(() => {
          // Token invalid or expired – clear it
          delete apiClient.defaults.headers.common['Authorization'];
          eraseCookie('jwt');
        })
        .finally(() => setLoading(false));
    } else {
      setLoading(false);
    }
  }, []);

  const login = useCallback(async (email, password) => {
    setError(null);
    const { token } = await loginUser({ email, password });
    apiClient.defaults.headers.common['Authorization'] = `Bearer ${token}`;
    setCookie('jwt', token);
    const userData = await loadUserDetails();
    setUser(userData);
    return userData;
  }, []);

  const register = useCallback(async (name, email, password, role) => {
    setError(null);
    const { token } = await registerUser({ name, email, password, role });
    apiClient.defaults.headers.common['Authorization'] = `Bearer ${token}`;
    setCookie('jwt', token);
    const userData = await loadUserDetails();
    setUser(userData);
    return userData;
  }, []);

  const logout = useCallback(async () => {
    try {
      await logoutUser();
    } catch (e) {
      console.error('Logout error:', e);
    }
    delete apiClient.defaults.headers.common['Authorization'];
    eraseCookie('jwt');
    setUser(null);
  }, []);

  const refreshUser = useCallback(async () => {
    const userData = await loadUserDetails();
    setUser(userData);
    return userData;
  }, []);

  const updateUser = useCallback(async (request) => {
    setError(null);
    await updateUserDetails(request);
    await refreshUser();
  }, [refreshUser]);

  const value = {
    user,
    loading,
    isAuthenticated,
    error,
    login,
    register,
    logout,
    refreshUser,
    updateUser,
  };

  return (
    <AuthContext.Provider value={value}>
      {children}
    </AuthContext.Provider>
  );
}
