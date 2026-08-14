const LoadingCard = ({ label = 'Loading…' }) => (
  <div className="card" style={{ padding: '48px', textAlign: 'center', color: 'var(--text-secondary)' }}>
    {label}
  </div>
);

export default LoadingCard;
