// STUB: This file is deprecated. API calls are now handled by RTK Query.
// TODO: Remove this file after full Redux migration.
const API_BASE_URL = import.meta.env.VITE_API_BASE_URL || 'http://localhost:8080';

const apiClient = {
  get: (url, config) => fetch(`${API_BASE_URL}${url}`, config).then(r => r.json()),
  post: (url, data, config) => fetch(`${API_BASE_URL}${url}`, { method: 'POST', body: JSON.stringify(data), headers: { 'Content-Type': 'application/json' }, ...config }).then(r => r.json()),
  put: (url, data, config) => fetch(`${API_BASE_URL}${url}`, { method: 'PUT', body: JSON.stringify(data), headers: { 'Content-Type': 'application/json' }, ...config }).then(r => r.json()),
  delete: (url, config) => fetch(`${API_BASE_URL}${url}`, { method: 'DELETE', ...config }).then(r => r.json()),
};

export default apiClient;
