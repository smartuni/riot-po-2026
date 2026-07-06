import React from 'react';
import { useGetActivitiesQuery } from '../../../app/store/api/api';

const getDotClass = (type) => {
  switch (type) {
    case 'SENSOR_NEW':
      return 'blue';
    case 'SENSOR_VALUE_CHANGED':
      return 'amber';
    case 'SENSOR_VALUE_KEEPALIVE':
      return 'blue';
    case 'SENSEMATE_WORKER_REPORT':
      return 'green';
    case 'TARGET_STATE_REQUEST':
      return 'amber';
    case 'MANUAL_STATUS_SET':
      return 'red';
    default:
      return 'blue';
  }
};

const getDotIcon = (type) => {
  switch (type) {
    case 'SENSOR_NEW':
      return '🆕';
    case 'SENSOR_VALUE_CHANGED':
      return '🔄';
    case 'SENSOR_VALUE_KEEPALIVE':
      return '📡';
    case 'SENSEMATE_WORKER_REPORT':
      return '👷';
    case 'TARGET_STATE_REQUEST':
      return '📋';
    case 'MANUAL_STATUS_SET':
      return '✋';
    default:
      return '📊';
  }
};

const formatTime = (timestamp) => {
  const diff = Date.now() - new Date(timestamp).getTime();
  const minutes = Math.floor(diff / 60000);
  if (minutes < 1) return 'just now';
  if (minutes < 60) return `${minutes}m ago`;
  const hours = Math.floor(minutes / 60);
  if (hours < 24) return `${hours}h ago`;
  const days = Math.floor(hours / 24);
  return `${days}d ago`;
};

export default function ActivityPanel() {
  const { data: activities = [], isLoading, error } = useGetActivitiesQuery();

  if (isLoading) {
    return (
      <div className="card activity-panel">
        <div className="card-header">
          <span className="card-title">Recent Activity</span>
        </div>
        <div className="activity-list" style={{ textAlign: 'center', padding: '40px 20px', color: 'var(--text-secondary)', fontSize: '13px' }}>
          Loading activities...
        </div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="card activity-panel">
        <div className="card-header">
          <span className="card-title">Recent Activity</span>
        </div>
        <div className="activity-list" style={{ textAlign: 'center', padding: '40px 20px', color: 'var(--red-500)', fontSize: '13px' }}>
          Failed to load activities: {error.toString()}
        </div>
      </div>
    );
  }

  // 7 most recent, reverse chronological
  const recent = [...activities]
    .sort((a, b) => new Date(b.lastTimeStamp) - new Date(a.lastTimeStamp))
    .slice(0, 7);

  return (
    <div className="card activity-panel">
      <div className="card-header">
        <span className="card-title">Recent Activity</span>
      </div>
      <div className="activity-list">
        {recent.length === 0 ? (
          <div style={{ textAlign: 'center', padding: '40px 20px', color: 'var(--text-secondary)', fontSize: '13px' }}>
            No recent activity
          </div>
        ) : (
          recent.map((activity, i) => (
            <div className="activity-item" key={activity.id || i}>
              <div className={`activity-dot ${getDotClass(activity.activityType)}`}>
                {getDotIcon(activity.activityType)}
              </div>
              <div>
                <div className="activity-text">
                  {activity.gateId ? <strong>{activity.gateId}</strong> : ''}{' '}
                  {activity.message || ''}
                </div>
                <div className="activity-time">
                  {activity.lastTimeStamp ? formatTime(activity.lastTimeStamp) : ''}
                </div>
              </div>
            </div>
          ))
        )}
      </div>
    </div>
  );
}
