export const statusInfo = (status) => {
  switch (status) {
    case 'OPEN': return { cls: 'status-open', label: 'Open' };
    case 'CLOSED': return { cls: 'status-closed', label: 'Closed' };
    default: return { cls: 'status-oos', label: 'OOS' };
  }
};

export const priorityClass = (level) => {
  switch (level) {
    case 0: return 'priority-low';
    case 1: return 'priority-medium';
    case 2: return 'priority-high';
    case 3: return 'priority-critical';
    default: return 'priority-low';
  }
};

export const confidenceColor = (conf) => {
  if (conf == null) return 'var(--text-secondary)';
  if (conf >= 90) return 'var(--green-600)';
  if (conf >= 70) return 'var(--amber-600)';
  return 'var(--red-600)';
};

export const stateConfirmationInfo = (sc) => {
  switch (sc) {
    case 'WORKER_CONFIRMED_SINGLE': return { cls: 'status-open', label: '1 worker' };
    case 'WORKER_CONFIRMED_MULTI': return { cls: 'status-closed', label: '2+ workers' };
    case 'WORKER_CONFIRMED_ALL': return { cls: 'status-closed', label: 'All workers' };
    case 'WORKER_CONFLICT': return { cls: 'status-oos', label: 'Conflict' };
    case 'UNCONFIRMED': return { cls: 'status-none', label: 'Unconfirmed' };
    default: return { cls: 'status-none', label: sc || '—' };
  }
};

export const getTimeAgo = (timestamp) => {
  const date = new Date(timestamp);
  const now = new Date();
  const secondsAgo = Math.floor((now - date) / 1000);
  if (secondsAgo < 60) return `${secondsAgo}s ago`;
  const minutes = Math.floor(secondsAgo / 60);
  if (minutes < 60) return `${minutes}m ago`;
  const hours = Math.floor(minutes / 60);
  if (hours < 24) return `${hours}h ago`;
  const days = Math.floor(hours / 24);
  if (days === 1) return 'yesterday';
  return `${days}d ago`;
};
