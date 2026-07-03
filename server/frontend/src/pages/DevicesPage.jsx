import { useState, useEffect } from 'react';
import AppLayout from '../features/shell/components/AppLayout';
import { RootKeySection, NodeTable } from '../features/nodes';

const DevicesPage = () => {
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

export default DevicesPage;
