import { useState, useEffect } from 'react';

export default function DarkModeToggle() {
  const [isDark, setIsDark] = useState(() => {
    return document.body.classList.contains('dark');
  });

  useEffect(() => {
    document.body.classList.toggle('dark', isDark);
    localStorage.setItem('sensemante-dark', isDark);
  }, [isDark]);

  return (
    <button
      className="topbar-btn dark-toggle"
      title="Toggle dark mode"
      onClick={() => setIsDark(!isDark)}
      type="button"
    >
      {isDark ? '☀️' : '🌙'}
    </button>
  );
}
