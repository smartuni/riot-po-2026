import { useEffect, useRef, useReducer } from 'react';
import { useAppSelector, useAppDispatch } from '../../app/store';
import { resetUplinkString } from '../../app/store/slices/gatesSlice';

const selectUplinkString = (s) => s.gates.uplinkString;

const TOAST_DURATION = 5000;
const EXIT_ANIM_MS = 300;

const initialState = { show: false, message: '', exiting: false };

function toastReducer(state, action) {
  switch (action.type) {
    case 'SHOW':
      return { show: true, message: action.payload, exiting: false };
    case 'EXIT':
      return { ...state, exiting: true };
    case 'HIDE':
      return { show: false, message: '', exiting: false };
    default:
      return state;
  }
}

export default function UplinkToast() {
  const dispatch = useAppDispatch();
  const uplinkString = useAppSelector(selectUplinkString);
  const [toastState, toastDispatch] = useReducer(toastReducer, initialState);
  const timerRef = useRef(null);
  const exitTimerRef = useRef(null);

  // Cleanup timers on unmount
  useEffect(() => {
    return () => {
      if (timerRef.current) clearTimeout(timerRef.current);
      if (exitTimerRef.current) clearTimeout(exitTimerRef.current);
    };
  }, []);

  // React to Redux uplinkString changes
  useEffect(() => {
    if (!uplinkString) return;

    // Clear any pending timers
    if (timerRef.current) clearTimeout(timerRef.current);
    if (exitTimerRef.current) clearTimeout(exitTimerRef.current);

    toastDispatch({ type: 'SHOW', payload: uplinkString });

    timerRef.current = setTimeout(() => {
      toastDispatch({ type: 'EXIT' });
      exitTimerRef.current = setTimeout(() => {
        toastDispatch({ type: 'HIDE' });
        dispatch(resetUplinkString());
      }, EXIT_ANIM_MS);
    }, TOAST_DURATION);
  }, [uplinkString, dispatch]);

  const handleDismiss = () => {
    if (timerRef.current) clearTimeout(timerRef.current);
    if (exitTimerRef.current) clearTimeout(exitTimerRef.current);

    toastDispatch({ type: 'EXIT' });
    exitTimerRef.current = setTimeout(() => {
      toastDispatch({ type: 'HIDE' });
      dispatch(resetUplinkString());
    }, EXIT_ANIM_MS);
  };

  if (!toastState.show) return null;

  return (
    <div className={`uplink-toast ${toastState.exiting ? 'hiding' : 'visible'}`}>
      <div className="uplink-toast-header">
        <span className="uplink-toast-title">
          <span className="uplink-toast-icon">&#x1F4E1;</span>
          Uplink Received
        </span>
        <button
          className="uplink-toast-close"
          onClick={handleDismiss}
          aria-label="Dismiss"
        >
          &times;
        </button>
      </div>
      <div className="uplink-toast-body">{toastState.message}</div>
    </div>
  );
}
