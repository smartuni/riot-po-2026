import React from 'react';
import '../styles/theme.css';

export default function ComingSoonHero({ icon, title, subtitle }) {
  return (
    <div className="coming-soon-hero">
      <div className="cs-icon">{icon}</div>
      <h2>{title}</h2>
      <p>{subtitle}</p>
    </div>
  );
}
