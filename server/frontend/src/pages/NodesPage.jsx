import { useState, useEffect } from 'react';
import AppLayout from '../features/shell/components/AppLayout';
import { RootKeySection, NodeTable } from '../features/nodes';

const NodesPage = () => {
  const [isNarrow, setIsNarrow] = useState(
    typeof window !== 'undefined'
      ? window.matchMedia('(max-width: 900px)').matches
      : false
  );

  useEffect(() => {
    const mql = window.matchMedia('(max-width: 900px)');
    const handler = (e) => setIsNarrow(e.matches);
    mql.addEventListener('change', handler);
    return () => mql.removeEventListener('change', handler);
  }, []);

  return (
    <AppLayout>
      <div style={{ marginBottom: '24px' }}>
        <h1 style={{ fontSize: '24px', fontWeight: 700, margin: 0 }}>Node Management</h1>
        <p style={{ fontSize: '14px', color: 'var(--text-secondary)', marginTop: '4px' }}>
          Manage root keys and registered nodes
        </p>
      </div>
      <div
        style={{
          display: 'grid',
          gridTemplateColumns: isNarrow ? '1fr' : 'minmax(0, 2fr) minmax(0, 1fr)',
          gap: '24px',
          alignItems: 'start',
        }}
      >
        <NodeTable />
        <RootKeySection />
      </div>
    </AppLayout>
  );
};

export default NodesPage;
