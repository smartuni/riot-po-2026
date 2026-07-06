import React from 'react';

export default function ComingSoonHero({ icon, title, subtitle }) {
  return (
    <div className="coming-soon-hero">
      <div className="cs-icon">{icon}</div>
      <h2>{title}</h2>
      <p>{subtitle}</p>
    </div>
  );
}
