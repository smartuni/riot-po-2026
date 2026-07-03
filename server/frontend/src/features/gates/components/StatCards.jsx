import React, { useState, useEffect, useRef } from "react";
import { useGetGatesQuery } from "../../../app/store/api/api";

function useCountUp(target) {
  const [display, setDisplay] = useState(0);
  const prevTarget = useRef(0);
  const rafRef = useRef(null);

  useEffect(() => {
    if (target === prevTarget.current) return;
    const start = prevTarget.current;
    const end = target;
    const duration = 600;
    const startTime = performance.now();

    const animate = (now) => {
      const elapsed = now - startTime;
      const progress = Math.min(elapsed / duration, 1);
      const eased = 1 - (1 - progress) * (1 - progress);
      setDisplay(Math.round(start + (end - start) * eased));
      if (progress < 1) {
        rafRef.current = requestAnimationFrame(animate);
      }
    };

    rafRef.current = requestAnimationFrame(animate);
    prevTarget.current = end;

    return () => {
      if (rafRef.current) cancelAnimationFrame(rafRef.current);
    };
  }, [target]);

  return display;
}

export default function StatCards({ filter = "", onFilter }) {
  const { data: gates = [], isLoading, error } = useGetGatesQuery();

  const total = gates.length;
  const closed = gates.filter(g => g.status === 'CLOSED').length;
  const open = gates.filter(g => g.status === 'OPEN').length;
  const oos = gates.filter(g => g.status === 'OUT_OF_SERVICE').length;

  const displayTotal = useCountUp(total);
  const displayClosed = useCountUp(closed);
  const displayOpen = useCountUp(open);
  const displayOos = useCountUp(oos);

  const handleToggle = (value) => {
    if (!onFilter) return;
    onFilter(filter === value ? "" : value);
  };

  const handleKeyDown = (e, value) => {
    if (e.key === 'Enter' || e.key === ' ') {
      e.preventDefault();
      handleToggle(value);
    }
  };

  const activeStyle = (value) => ({
    cursor: onFilter ? 'pointer' : 'default',
    outline: filter === value ? '2px solid var(--blue-600)' : 'none',
    outlineOffset: '-2px',
  });

  if (isLoading) {
    return (
      <div className="stats-grid">
        {[1, 2, 3, 4].map(i => (
          <div key={i} className="stat-card" style={{ opacity: 0.5 }}>
            <div className="stat-header">
              <span className="stat-label">Loading…</span>
            </div>
            <div className="stat-number">—</div>
          </div>
        ))}
      </div>
    );
  }

  if (error) {
    return (
      <div className="stats-grid">
        <div className="stat-card" style={{ gridColumn: '1 / -1', textAlign: 'center', padding: '24px', color: 'var(--red-600)' }}>
          Failed to load gate statistics.
        </div>
      </div>
    );
  }

  return (
    <div className="stats-grid">
      <div
        className="stat-card blue"
        role="button"
        tabIndex={0}
        style={activeStyle("")}
        onClick={() => handleToggle("")}
        onKeyDown={(e) => handleKeyDown(e, "")}
      >
        <div className="stat-header">
          <span className="stat-label">Total Gates</span>
          <div className="stat-icon blue">🚪</div>
        </div>
        <div className="stat-number" data-count={total}>{displayTotal}</div>
      </div>
      <div
        className="stat-card green"
        role="button"
        tabIndex={0}
        style={activeStyle("CLOSED")}
        onClick={() => handleToggle("CLOSED")}
        onKeyDown={(e) => handleKeyDown(e, "CLOSED")}
      >
        <div className="stat-header">
          <span className="stat-label">Closed</span>
          <div className="stat-icon green">✓</div>
        </div>
        <div className="stat-number" data-count={closed}>{displayClosed}</div>
      </div>
      <div
        className="stat-card red"
        role="button"
        tabIndex={0}
        style={activeStyle("OPEN")}
        onClick={() => handleToggle("OPEN")}
        onKeyDown={(e) => handleKeyDown(e, "OPEN")}
      >
        <div className="stat-header">
          <span className="stat-label">Open</span>
          <div className="stat-icon red">⚠</div>
        </div>
        <div className="stat-number" data-count={open}>{displayOpen}</div>
      </div>
      <div
        className="stat-card amber"
        role="button"
        tabIndex={0}
        style={activeStyle("OUT_OF_SERVICE")}
        onClick={() => handleToggle("OUT_OF_SERVICE")}
        onKeyDown={(e) => handleKeyDown(e, "OUT_OF_SERVICE")}
      >
        <div className="stat-header">
          <span className="stat-label">Out of Service</span>
          <div className="stat-icon amber">⏸</div>
        </div>
        <div className="stat-number" data-count={oos}>{displayOos}</div>
      </div>
    </div>
  );
}
