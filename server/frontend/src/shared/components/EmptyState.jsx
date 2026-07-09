const EmptyState = ({ icon, title, subtitle }) => (
  <div style={{ padding: '48px 24px', textAlign: 'center' }}>
    {icon && (
      <div
        style={{
          width: '48px',
          height: '48px',
          margin: '0 auto 16px',
          borderRadius: '12px',
          background: 'var(--blue-50)',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          color: 'var(--blue-600)',
        }}
      >
        {icon}
      </div>
    )}
    <p style={{ fontSize: '15px', fontWeight: 600, color: 'var(--text)', marginBottom: '4px' }}>
      {title}
    </p>
    {subtitle && (
      <p style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>
        {subtitle}
      </p>
    )}
  </div>
);

export default EmptyState;
