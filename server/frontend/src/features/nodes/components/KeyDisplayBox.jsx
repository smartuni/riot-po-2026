const KeyDisplayBox = ({ label, value, actionLabel, onAction }) => (
  <div className="form-group" style={{ marginBottom: '0' }}>
    <label className="form-label">{label}</label>
    <div
      style={{
        display: 'flex',
        alignItems: 'center',
        gap: '8px',
        padding: '8px 10px',
        background: 'var(--bg-sidebar)',
        border: '1px solid var(--border)',
        borderRadius: '8px',
      }}
    >
      <code
        className="mono"
        style={{
          fontSize: '12px',
          color: 'var(--text-secondary)',
          flex: 1,
          overflow: 'hidden',
          textOverflow: 'ellipsis',
          whiteSpace: 'nowrap',
        }}
      >
        {value}
      </code>
      <button
        type="button"
        className="btn btn-ghost"
        style={{ padding: '4px 10px', fontSize: '11px', flexShrink: 0 }}
        onClick={onAction}
      >
        {actionLabel}
      </button>
    </div>
  </div>
);

export default KeyDisplayBox;
