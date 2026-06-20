import React from 'react';
import AppLayout from '../features/shell/components/AppLayout';
import ComingSoonHero from '../shared/components/ComingSoonHero';
import { SkeletonLine, SkeletonCircle, SkeletonChart } from '../shared/components/SkeletonLoader';

const GATE_CARDS = 4;
const RULE_WIDTHS = ['90%', '70%', '50%', '80%'];

const AutomationPage = () => (
  <AppLayout>
    <ComingSoonHero
      icon="⚡"
      title="Gate Control"
      subtitle="Enter water level to see which gates should be closed"
    />

    {/* ── Skeleton water-level input ──────────────────────── */}
    <div className="coming-soon-grid">
      <div className="skeleton-card">
        <SkeletonCircle size={40} />
        <SkeletonLine width="60%" />
        <SkeletonLine width="40%" />
      </div>
      {Array.from({ length: GATE_CARDS }).map((_, i) => (
        <div key={i} className="skeleton-card">
          <SkeletonCircle size={12} />
          <SkeletonLine width="70%" />
          <SkeletonLine width="50%" />
          <SkeletonLine width="30%" />
        </div>
      ))}
    </div>

    {/* ── Skeleton chart ──────────────────────────────────── */}
    <SkeletonChart height={180} />

    {/* ── Skeleton rules ──────────────────────────────────── */}
    <div style={{ display: 'flex', flexDirection: 'column', gap: 10, marginTop: 24 }}>
      {RULE_WIDTHS.map((w, i) => (
        <SkeletonLine key={i} width={w} />
      ))}
    </div>
  </AppLayout>
);

export default AutomationPage;
