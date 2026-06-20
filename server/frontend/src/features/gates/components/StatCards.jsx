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
      // ease-out quad
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

export default function StatCards() {
  const { data: gates = [] } = useGetGatesQuery();

  const total = gates.length;
  const closed = gates.filter(g => g.status === 'CLOSED').length;
  const open = gates.filter(g => g.status === 'OPEN').length;
  const oos = gates.filter(g => g.status === 'OUT_OF_SERVICE').length;

  const displayTotal = useCountUp(total);
  const displayClosed = useCountUp(closed);
  const displayOpen = useCountUp(open);
  const displayOos = useCountUp(oos);

  return (
    <div className="stats-grid">
      <div className="stat-card blue">
        <div className="stat-header">
          <span className="stat-label">Total Gates</span>
          <div className="stat-icon blue">🚪</div>
        </div>
        <div className="stat-number" data-count={total}>{displayTotal}</div>
      </div>
      <div className="stat-card green">
        <div className="stat-header">
          <span className="stat-label">Closed</span>
          <div className="stat-icon green">✓</div>
        </div>
        <div className="stat-number" data-count={closed}>{displayClosed}</div>
      </div>
      <div className="stat-card red">
        <div className="stat-header">
          <span className="stat-label">Open</span>
          <div className="stat-icon red">⚠</div>
        </div>
        <div className="stat-number" data-count={open}>{displayOpen}</div>
      </div>
      <div className="stat-card amber">
        <div className="stat-header">
          <span className="stat-label">Out of Service</span>
          <div className="stat-icon amber">⏸</div>
        </div>
        <div className="stat-number" data-count={oos}>{displayOos}</div>
      </div>
    </div>
  );
}
