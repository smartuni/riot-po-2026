import { useState, useMemo } from 'react';
import AppLayout from '../features/shell/components/AppLayout';
import MapView from '../features/map/components/MapView';
import { useGetGatesQuery } from '../app/store/api/api';

const STATUS_LABELS = ['Closed', 'Open', 'OOS'];

const STATUS_COLORS = {
  Closed: 'var(--green-600)',
  Open: 'var(--red-600)',
  OOS: 'var(--amber-600)',
};

const MapPage = () => {
  const [search, setSearch] = useState('');
  const [filters, setFilters] = useState({
    Closed: true,
    Open: true,
    OOS: true,
  });

  const { data: gates = [], isLoading, error } = useGetGatesQuery();

  const counts = useMemo(() => {
    const total = gates.length;
    const closed = gates.filter((g) => g.status === 'CLOSED').length;
    const open = gates.filter((g) => g.status === 'OPEN').length;
    const oos = gates.filter((g) => g.status === 'OUT_OF_SERVICE').length;
    return { total, closed, open, oos };
  }, [gates]);

  const handleFilterChange = (label) => {
    setFilters((prev) => ({ ...prev, [label]: !prev[label] }));
  };

  return (
    <AppLayout>
      <div style={{ position: 'relative', height: 'calc(100vh - 60px)', padding: 0 }}>
        <div className="map-page-container" style={{ height: '100%', width: '100%' }}>
            <MapView statusFilter={filters} search={search} />
          </div>

          {/* Floating filter panel — top-right */}
          <div className="map-filter-panel">
            <h4>Filter by Status</h4>
            <div style={{ marginBottom: 12 }}>
              <input
                className="form-input"
                type="text"
                placeholder="Search by ID or location…"
                value={search}
                onChange={(e) => setSearch(e.target.value)}
                style={{ width: '100%', fontSize: 13 }}
              />
            </div>
            {STATUS_LABELS.map((label) => (
              <label key={label} className="map-filter-item">
                <input
                  type="checkbox"
                  checked={filters[label]}
                  onChange={() => handleFilterChange(label)}
                />
                {label}
                <span
                  style={{
                    color: STATUS_COLORS[label],
                    fontSize: '11px',
                    marginLeft: 'auto',
                    fontFamily: '"JetBrains Mono", monospace',
                  }}
                >
                  {counts[label.toLowerCase()]}
                </span>
              </label>
            ))}
          </div>

          {/* Floating summary card — bottom-left */}
          <div className="map-summary">
            {isLoading ? (
              <div style={{ color: 'var(--text-secondary)', fontSize: 13 }}>Loading…</div>
            ) : error ? (
              <div style={{ color: 'var(--red-600)', fontSize: 13 }}>Failed to load</div>
            ) : (
              <>
                <div className="map-stat">
                  <div className="num" style={{ color: 'var(--blue-600)' }}>
                    {counts.total}
                  </div>
                  <div className="lbl">Total</div>
                </div>
                <div className="map-stat">
                  <div className="num" style={{ color: 'var(--green-600)' }}>
                    {counts.closed}
                  </div>
                  <div className="lbl">Closed</div>
                </div>
                <div className="map-stat">
                  <div className="num" style={{ color: 'var(--red-600)' }}>
                    {counts.open}
                  </div>
                  <div className="lbl">Open</div>
                </div>
                <div className="map-stat">
                  <div className="num" style={{ color: 'var(--amber-600)' }}>
                    {counts.oos}
                  </div>
                  <div className="lbl">OOS</div>
                </div>
              </>
            )}
          </div>

          {/* Attribution — bottom-right */}
          <div className="map-attribution">Map data &copy; OpenStreetMap</div>
        </div>
      </AppLayout>
  );
};

export default MapPage;
