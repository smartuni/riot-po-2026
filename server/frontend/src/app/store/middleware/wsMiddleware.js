import SockJS from 'sockjs-client';
import { Client } from '@stomp/stompjs';
import {
  gateAdded,
  gateDeleted,
  gateUpdated,
  uplinkReceived,
} from '../slices/gatesSlice';
import {
  activityAdded,
  activityDeleted,
} from '../slices/activitiesSlice';

export const APP_START = 'ws/APP_START';
export const APP_STOP = 'ws/APP_STOP';

const INITIAL_RECONNECT_DELAY = 1000;
const MAX_RECONNECT_DELAY = 30000;

let stompClient = null;
let reconnectAttempts = 0;
let reconnectTimer = null;
let intentionalDisconnect = false;
let reconnecting = false;

function getReconnectDelay() {
  const delay = Math.min(
    INITIAL_RECONNECT_DELAY * Math.pow(2, reconnectAttempts),
    MAX_RECONNECT_DELAY
  );
  reconnectAttempts++;
  return delay;
}

function resetReconnectState() {
  reconnectAttempts = 0;
  reconnecting = false;
  if (reconnectTimer) {
    clearTimeout(reconnectTimer);
    reconnectTimer = null;
  }
}

function scheduleReconnect(store) {
  if (intentionalDisconnect || reconnecting) return;
  reconnecting = true;
  const delay = getReconnectDelay();
  reconnectTimer = setTimeout(() => {
    reconnecting = false;
    connect(store);
  }, delay);
}

function connect(store) {
  intentionalDisconnect = false;
  const wsUrl = import.meta.env.VITE_WS_URL;

  stompClient = new Client({
    webSocketFactory: () => new SockJS(wsUrl),
    reconnectDelay: 0,
    onConnect: () => {
      resetReconnectState();

      stompClient.subscribe('/topic/gates/add', (message) => {
        store.dispatch(gateAdded(JSON.parse(message.body)));
      });

      stompClient.subscribe('/topic/gates/delete', (message) => {
        store.dispatch(gateDeleted(message.body));
      });

      stompClient.subscribe('/topic/gates/updates', (message) => {
        store.dispatch(gateUpdated(JSON.parse(message.body)));
      });

      stompClient.subscribe('/topic/gate-activities', (message) => {
        store.dispatch(activityAdded(JSON.parse(message.body)));
      });

      stompClient.subscribe('/topic/gate-activities/delete', (message) => {
        store.dispatch(activityDeleted(message.body));
      });

      stompClient.subscribe('/topic/uplinks', (message) => {
        store.dispatch(uplinkReceived(message.body));
      });
    },
    onStompError: (frame) => {
      console.error('STOMP error:', frame.headers?.message || frame.body);
      scheduleReconnect(store);
    },
    onWebSocketClose: () => {
      scheduleReconnect(store);
    },
  });

  stompClient.activate();
}

function disconnect() {
  intentionalDisconnect = true;
  resetReconnectState();
  if (stompClient) {
    stompClient.deactivate();
    stompClient = null;
  }
}

const wsMiddleware = (store) => (next) => (action) => {
  if (action.type === APP_START && !stompClient) {
    connect(store);
  }
  if (action.type === APP_STOP) {
    disconnect();
  }
  return next(action);
};

export default wsMiddleware;
