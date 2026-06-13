import React from 'react';
import AppLayout from '../features/shell/components/AppLayout';
import ComingSoonHero from '../shared/components/ComingSoonHero';
import { SkeletonLine, SkeletonCircle, SkeletonChart } from '../shared/components/SkeletonLoader';
import '../shared/styles/theme.css';

const DiagnosticsPage = () => {
  const healthCards = Array.from({ length: 4 });

  const errorLogWidths = ['90%', '70%', '50%', '80%', '60%'];

  return (
    <AppLayout>
        <ComingSoonHero
          icon="🔍"
          title="Diagnostics"
          subtitle="Deep system health monitoring and performance analytics"
        />

        {/* ── Skeleton health cards ────────────────────────────── */}
        <div className="coming-soon-grid">
          {healthCards.map((_, i) => (
            <div key={i} className="skeleton-card">
              <SkeletonCircle size={40} />
              <SkeletonLine width="70%" />
              <SkeletonLine width="40%" />
            </div>
          ))}
        </div>

        {/* ── Skeleton chart ───────────────────────────────────── */}
        <SkeletonChart height={200} />

        {/* ── Skeleton error log ────────────────────────────────── */}
        <div style={{ display: 'flex', flexDirection: 'column', gap: 10, marginTop: 24 }}>
          {errorLogWidths.map((w, i) => (
            <SkeletonLine key={i} width={w} />
          ))}
        </div>
      </AppLayout>
  );
};

export default DiagnosticsPage;
