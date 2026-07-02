import { useAppDispatch, useAppSelector } from '../../app/store';
import { toggleDarkMode } from '../../app/store/slices/uiSlice';

export default function DarkModeToggle() {
  const isDark = useAppSelector((state) => state.ui.darkMode);
  const dispatch = useAppDispatch();

  return (
    <button
      className="topbar-btn dark-toggle"
      title="Toggle dark mode"
      aria-label="Toggle dark mode"
      onClick={() => dispatch(toggleDarkMode())}
      type="button"
    >
      {isDark ? '☀️' : '🌙'}
    </button>
  );
}
