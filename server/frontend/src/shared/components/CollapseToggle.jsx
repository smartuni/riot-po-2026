export default function CollapseToggle({ collapsed, onToggle }) {
  return (
    <button
      className="collapse-btn topbar-btn"
      title="Toggle sidebar"
      aria-label="Toggle sidebar"
      onClick={onToggle}
      type="button"
    >
      {collapsed ? '☰' : '✕'}
    </button>
  );
}
