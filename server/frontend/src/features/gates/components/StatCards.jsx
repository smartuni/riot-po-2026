import React from "react";
import { useGetGatesQuery } from "../../../app/store/api/api";

export default function StatCards() {
  const { data: gates = [] } = useGetGatesQuery();

  const total = gates.length;
  const closed = gates.filter(g => g.status === 'CLOSED').length;
  const open = gates.filter(g => g.status === 'OPEN').length;
  const oos = gates.filter(g => g.status === 'OUT_OF_SERVICE').length;

  return (
    <div className="stats-grid">
      <div className="stat-card blue">
        <div className="stat-header">
          <span className="stat-label">Total Gates</span>
          <div className="stat-icon blue">🚪</div>
        </div>
        <div className="stat-number" data-count={total}>{total}</div>
      </div>
      <div className="stat-card green">
        <div className="stat-header">
          <span className="stat-label">Closed</span>
          <div className="stat-icon green">✓</div>
        </div>
        <div className="stat-number" data-count={closed}>{closed}</div>
      </div>
      <div className="stat-card red">
        <div className="stat-header">
          <span className="stat-label">Open</span>
          <div className="stat-icon red">⚠</div>
        </div>
        <div className="stat-number" data-count={open}>{open}</div>
      </div>
      <div className="stat-card amber">
        <div className="stat-header">
          <span className="stat-label">Out of Service</span>
          <div className="stat-icon amber">⏸</div>
        </div>
        <div className="stat-number" data-count={oos}>{oos}</div>
      </div>
    </div>
  );
}
