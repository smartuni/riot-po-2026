import React, { useState, useMemo } from 'react';
import AppLayout from '../features/shell/components/AppLayout';
import { useGetActivitiesQuery } from '../app/store/api/api';

const getDotClass = (type) => {
  switch (type) {
    case 'SENSOR_NEW': return 'blue';
    case 'SENSOR_VALUE_CHANGED': return 'amber';
    case 'SENSOR_VALUE_KEEPALIVE': return 'blue';
    case 'SENSEMATE_WORKER_REPORT': return 'green';
    case 'TARGET_STATE_REQUEST': return 'amber';
    case 'MANUAL_STATUS_SET': return 'red';
    default: return 'blue';
  }
};

const getDotIcon = (type) => {
  switch (type) {
    case 'SENSOR_NEW': return '🆕';
    case 'SENSOR_VALUE_CHANGED': return '🔄';
    case 'SENSOR_VALUE_KEEPALIVE': return '📡';
    case 'SENSEMATE_WORKER_REPORT': return '👷';
    case 'TARGET_STATE_REQUEST': return '📋';
    case 'MANUAL_STATUS_SET': return '✋';
    default: return '📊';
  }
};

const formatTimestamp = (ts) => {
  if (!ts) return '—';
  const d = new Date(ts);
  return d.toLocaleString(undefined, {
    year: 'numeric', month: 'short', day: '2-digit',
    hour: '2-digit', minute: '2-digit', second: '2-digit',
  });
};

export default function LogsPage() {
  const { data: activities = [], isLoading, error } = useGetActivitiesQuery();
  const [filter, setFilter] = useState('');

  const filtered = useMemo(() => {
    const sorted = [...activities].sort(
      (a, b) => new Date(b.lastTimeStamp) - new Date(a.lastTimeStamp)
    );
    if (!filter.trim()) return sorted;
    const q = filter.toLowerCase();
    return sorted.filter(a => {
      const fields = [
        a.gateId, a.message, a.activityType,
        a.lastTimeStamp ? new Date(a.lastTimeStamp).toLocaleString() : '',
      ];
      return fields.some(f => f != null && String(f).toLowerCase().includes(q));
    });
  }, [activities, filter]);

  return (
    <AppLayout>
      <div className="page-content-inner">
        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: 20, flexWrap: 'wrap', gap: 12 }}>
          <h2 style={{ margin: 0, fontSize: 22, fontWeight: 700 }}>Activity Logs</h2>
          <input
            className="search-input"
            type="text"
            aria-label="Filter logs"
            placeholder="Filter by gate, message, or type…"
            value={filter}
            onChange={e => setFilter(e.target.value)}
            style={{ maxWidth: 360, width: '100%' }}
          />
        </div>

        {isLoading && (
          <div style={{ textAlign: 'center', padding: '40px 20px', color: 'var(--text-secondary)' }}>
            Loading logs…
          </div>
        )}

        {error && (
          <div style={{ textAlign: 'center', padding: '40px 20px', color: 'var(--red-500)' }}>
            Failed to load logs: {error.toString()}
          </div>
        )}

        {!isLoading && !error && (
          <div className="card" style={{ padding: 0, overflow: 'auto' }}>
            <table style={{ width: '100%', borderCollapse: 'collapse', fontSize: 14 }}>
              <thead>
                <tr style={{ borderBottom: '2px solid var(--border)' }}>
                  <th style={{ textAlign: 'left', padding: '10px 16px', color: 'var(--text-secondary)', fontWeight: 600 }}>Type</th>
                  <th style={{ textAlign: 'left', padding: '10px 16px', color: 'var(--text-secondary)', fontWeight: 600 }}>Gate</th>
                  <th style={{ textAlign: 'left', padding: '10px 16px', color: 'var(--text-secondary)', fontWeight: 600 }}>Message</th>
                  <th style={{ textAlign: 'left', padding: '10px 16px', color: 'var(--text-secondary)', fontWeight: 600 }}>Time</th>
                </tr>
              </thead>
              <tbody>
                {filtered.length === 0 ? (
                  <tr>
                    <td colSpan={4} style={{ textAlign: 'center', padding: '40px 20px', color: 'var(--text-secondary)' }}>
                      {filter ? 'No logs match your filter' : 'No activity logs yet'}
                    </td>
                  </tr>
                ) : (
                  filtered.map((a, i) => (
                    <tr key={a.id || i} style={{ borderBottom: '1px solid var(--border)' }}>
                      <td style={{ padding: '10px 16px' }}>
                        <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
                          <span className={`activity-dot ${getDotClass(a.activityType)}`} style={{ width: 22, height: 22, fontSize: 11 }}>
                            {getDotIcon(a.activityType)}
                          </span>
                          <span style={{ color: 'var(--text-secondary)', fontSize: 13 }}>{a.activityType || '—'}</span>
                        </div>
                      </td>
                      <td style={{ padding: '10px 16px', fontWeight: 600 }}>{a.gateId || '—'}</td>
                      <td style={{ padding: '10px 16px' }}>{a.message || '—'}</td>
                      <td style={{ padding: '10px 16px', color: 'var(--text-secondary)', whiteSpace: 'nowrap', fontSize: 13 }}>
                        {formatTimestamp(a.lastTimeStamp)}
                      </td>
                    </tr>
                  ))
                )}
              </tbody>
            </table>
          </div>
        )}
      </div>
    </AppLayout>
  );
}
