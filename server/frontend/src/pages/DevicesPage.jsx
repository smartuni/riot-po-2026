import React from 'react';
import { useNavigate } from 'react-router-dom';
import AppLayout from '../features/shell/components/AppLayout';
import { useGetGatesQuery } from '../app/store/api/api';

const statusInfo = (status) => {
  switch (status) {
    case 'OPEN': return { cls: 'status-open', label: 'Open' };
    case 'CLOSED': return { cls: 'status-closed', label: 'Closed' };
    default: return { cls: 'status-oos', label: 'OOS' };
  }
};

const priorityClass = (level) => {
  switch (level) {
    case 0: return 'priority-low';
    case 1: return 'priority-medium';
    case 2: return 'priority-high';
    case 3: return 'priority-critical';
    default: return 'priority-low';
  }
};

const confidenceColor = (conf) => {
  if (conf == null) return 'var(--text-secondary)';
  if (conf >= 90) return 'var(--green-600)';
  if (conf >= 70) return 'var(--amber-600)';
  return 'var(--red-600)';
};

const stateConfirmationInfo = (sc) => {
  switch (sc) {
    case 'WORKER_CONFIRMED_SINGLE': return { cls: 'status-open', label: '1 worker' };
    case 'WORKER_CONFIRMED_MULTI': return { cls: 'status-closed', label: '2+ workers' };
    case 'WORKER_CONFIRMED_ALL': return { cls: 'status-closed', label: 'All workers' };
    case 'WORKER_CONFLICT': return { cls: 'status-oos', label: 'Conflict' };
    case 'UNCONFIRMED': return { cls: 'status-none', label: 'Unconfirmed' };
    default: return { cls: 'status-none', label: sc || '—' };
  }
};

function getTimeAgo(timestamp) {
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
}

const DevicesPage = () => {
  const navigate = useNavigate();
  const { data: gates = [], isLoading, error } = useGetGatesQuery();

  if (isLoading) {
    return (
      <AppLayout>
        <div className="skeleton-grid">
          {Array.from({ length: 4 }).map((_, i) => (
            <div key={i} className="skeleton-card">
              <div className="skeleton-circle" style={{ width: 24, height: 24 }} />
              <div className="skeleton-line medium" />
              <div className="skeleton-line long" />
              <div className="skeleton-line short" />
            </div>
          ))}
        </div>
      </AppLayout>
    );
  }

  if (error) {
    return (
      <AppLayout>
        <div className="card" style={{ padding: '24px', textAlign: 'center' }}>
          <p style={{ fontSize: '14px', color: 'var(--text-secondary)' }}>
            Failed to load gates. Please try again later.
          </p>
        </div>
      </AppLayout>
    );
  }

  return (
    <AppLayout>
      <div style={{ marginBottom: '24px' }}>
        <h1 style={{ fontSize: '24px', fontWeight: 700, margin: 0 }}>Gate Overview</h1>
        <p style={{ fontSize: '14px', color: 'var(--text-secondary)', marginTop: '4px' }}>
          All gates and their key information
        </p>
      </div>
      <div className="gate-overview-grid">
        {gates.map((gate) => {
          const si = statusInfo(gate.status);
          const pc = priorityClass(gate.priority);
          const cc = confidenceColor(gate.confidence);
          const stateConf = stateConfirmationInfo(gate.stateConfirmation);
          return (
            <div
              key={gate.id}
              className="gate-overview-card"
              onClick={() => navigate(`/gates/${gate.id}`)}
            >
              <div className="gate-overview-card-header">
                <div style={{ display: 'flex', alignItems: 'center', gap: '8px', minWidth: 0 }}>
                  <span className="gate-id">#{gate.id}</span>
                  <span
                    style={{
                      fontSize: '14px',
                      fontWeight: 600,
                      color: 'var(--text)',
                      overflow: 'hidden',
                      textOverflow: 'ellipsis',
                      whiteSpace: 'nowrap',
                    }}
                  >
                    {gate.location || 'Unnamed'}
                  </span>
                </div>
                <div style={{ display: 'flex', alignItems: 'center', gap: '6px', flexShrink: 0 }}>
                  {gate.manualOverride && <span className="manual-pill">Manual</span>}
                  <span className={`status-badge ${si.cls}`}>
                    <span className="status-dot" />
                    {si.label}
                  </span>
                </div>
              </div>
              <div className="gate-overview-card-body">
                <div>
                  <span className="gate-overview-info-label">Confidence</span>
                  <span className="gate-overview-info-value" style={{ display: 'inline-flex', alignItems: 'center', gap: '6px' }}>
                    <span
                      style={{
                        width: '8px',
                        height: '8px',
                        borderRadius: '50%',
                        background: cc,
                        display: 'inline-block',
                        flexShrink: 0,
                      }}
                    />
                    {gate.confidence != null ? `${gate.confidence}%` : '—'}
                  </span>
                </div>
                <div>
                  <span className="gate-overview-info-label">Height above NN</span>
                  <span className="gate-overview-info-value">
                    {gate.heightAboveNN != null ? `${gate.heightAboveNN} m` : '—'}
                  </span>
                </div>
                <div className={pc}>
                  <span className="gate-overview-info-label">Priority</span>
                  <span className="gate-overview-info-value" style={{ display: 'inline-flex', alignItems: 'center' }}>
                    <span className="priority-dot" />
                    {gate.priority != null ? gate.priority : '—'}
                  </span>
                </div>
                <div>
                  <span className="gate-overview-info-label">Last Update</span>
                  <span className="gate-overview-info-value last-update">
                    {gate.lastTimeStamp ? getTimeAgo(gate.lastTimeStamp) : '—'}
                  </span>
                </div>
                <div>
                  <span className="gate-overview-info-label">State Confirmation</span>
                  <span className={`status-badge ${stateConf.cls}`} style={{ marginTop: '2px' }}>
                    <span className="status-dot" />
                    {stateConf.label}
                  </span>
                </div>
              </div>
              <div className="gate-overview-card-footer">
                <span style={{ color: 'var(--text-secondary)' }}>
                  Device: <span className="gate-id">{gate.deviceId ?? '—'}</span>
                </span>
                <span className="action-link">View Details →</span>
              </div>
            </div>
          );
        })}
      </div>
    </AppLayout>
  );
};

export default DevicesPage;
