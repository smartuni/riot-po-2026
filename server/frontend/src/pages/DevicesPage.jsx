import React from 'react';
import { useNavigate } from 'react-router-dom';
import AppLayout from '../features/shell/components/AppLayout';
import { useGetGatesQuery } from '../app/store/api/api';
import { useAppSelector } from '../app/store';
import GateOverviewCard from '../features/gates/components/GateOverviewCard';
import { HealthBadge } from '../features/health';

const DevicesPage = () => {
  const navigate = useNavigate();
  const { data: gates = [], isLoading, error } = useGetGatesQuery();
  const healthBySenseGateId = useAppSelector((state) => state.health.bySenseGateId);

  const gateIds = new Set(gates.map((g) => g.id));
  const unmappedEntries = Object.entries(healthBySenseGateId).filter(
    ([senseGateId]) => !gateIds.has(Number(senseGateId))
  );

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
        {gates.map((gate) => (
          <GateOverviewCard
            key={gate.id}
            gate={gate}
            onClick={() => navigate(`/gates/${gate.id}`)}
          />
        ))}
      </div>
      {unmappedEntries.length > 0 && (
        <div style={{ marginTop: '24px' }}>
          <h2 style={{ fontSize: '20px', fontWeight: 700, margin: 0 }}>
            Unmapped Health Devices
          </h2>
          <p style={{ fontSize: '14px', color: 'var(--text-secondary)', marginTop: '4px' }}>
            Health data received for devices not in the gate list
          </p>
          <div className="gate-overview-grid">
            {unmappedEntries.map(([senseGateId, health]) => (
              <div key={senseGateId} className="gate-overview-card" style={{ opacity: 0.7 }}>
                <div className="gate-overview-card-header">
                  <span className="gate-id">#{senseGateId}</span>
                  <span style={{ fontSize: '14px', fontWeight: 600, color: 'var(--text-secondary)' }}>
                    Unmapped Device
                  </span>
                </div>
                <div className="gate-overview-card-body">
                  <div>
                    <span className="gate-overview-info-label">Health</span>
                    <HealthBadge health={health} />
                  </div>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}
    </AppLayout>
  );
};

export default DevicesPage;
