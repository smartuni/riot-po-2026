import React from 'react';
import {
  statusInfo,
  priorityClass,
  confidenceColor,
  stateConfirmationInfo,
  getTimeAgo,
} from './gateCardHelpers';
import { HealthBadge, useHealthForGate } from '../../health';

const GateOverviewCard = ({ gate, onClick }) => {
  const si = statusInfo(gate.status);
  const pc = priorityClass(gate.priority);
  const cc = confidenceColor(gate.confidence);
  const stateConf = stateConfirmationInfo(gate.stateConfirmation);
  const health = useHealthForGate(gate.id);

  return (
    <div className="gate-overview-card" onClick={onClick}>
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
        <div>
          <span className="gate-overview-info-label">Health</span>
          {/* TODO: verify senseGateId === gate.id mapping with firmware team */}
          <HealthBadge health={health} />
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
};

export default GateOverviewCard;
