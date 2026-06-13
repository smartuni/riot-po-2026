import React from 'react';
import '../styles/theme.css';

export function SkeletonLine({ width = '90%', className = '' }) {
  return <div className={`skeleton-line ${className}`.trim()} style={{ width }} />;
}

export function SkeletonCircle({ size = 40, className = '' }) {
  return (
    <div
      className={`skeleton-circle ${className}`.trim()}
      style={{ width: size, height: size }}
    />
  );
}

export function SkeletonChart({ height = 200, className = '' }) {
  return <div className={`skeleton-chart ${className}`.trim()} style={{ height }} />;
}
