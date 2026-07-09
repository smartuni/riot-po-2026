import { Client } from '@stomp/stompjs';
import { uplinkReceived } from '../slices/gatesSlice';
import { handleHealthMessage } from './healthMessageHandler';
import { healthReceived } from '../slices/healthSlice';
import { api } from '../api/api';

const INITIAL_RECONNECT_DELAY = 1000;
const MAX_RECONNECT_DELAY = 30000;

function createWsMiddleware() {
  let stompClient = null;
  let clientActive = false;
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
    if (stompClient) {
      stompClient.deactivate();
      stompClient = null;
    }
    intentionalDisconnect = false;
    const wsUrl = (window.location.protocol === 'https:' ? 'wss://' : 'ws://') + window.location.host + '/ws';

    stompClient = new Client({
      webSocketFactory: () => new WebSocket(wsUrl),
      reconnectDelay: 0,
      onConnect: () => {
        clientActive = true;
        resetReconnectState();

        stompClient.subscribe('/topic/gates/add', (message) => {
          let newGate;
          try {
            newGate = JSON.parse(message.body);
          } catch (err) {
            console.error('Failed to parse gates/add message:', err);
            return;
          }
          store.dispatch(
            api.util.updateQueryData('getGates', undefined, (draft) => {
              draft.push(newGate);
            })
          );
        });

        stompClient.subscribe('/topic/gates/delete', (message) => {
          const deletedGateId = parseInt(message.body);
          if (isNaN(deletedGateId)) {
            console.error('Failed to parse gates/delete message: invalid gate ID');
            return;
          }
          store.dispatch(
            api.util.updateQueryData('getGates', undefined, (draft) => {
              return draft.filter((gate) => gate.id !== deletedGateId);
            })
          );
        });

        stompClient.subscribe('/topic/gates/updates', (message) => {
          let updatedGate;
          try {
            updatedGate = JSON.parse(message.body);
          } catch (err) {
            console.error('Failed to parse gates/updates message:', err);
            return;
          }
          store.dispatch(
            api.util.updateQueryData('getGates', undefined, (draft) => {
              const idx = draft.findIndex((g) => g.id === updatedGate.id);
              if (idx !== -1) {
                draft[idx] = updatedGate;
              }
            })
          );
        });

        stompClient.subscribe('/topic/gate-activities', (message) => {
          let newActivity;
          try {
            newActivity = JSON.parse(message.body);
          } catch (err) {
            console.error('Failed to parse gate-activities message:', err);
            return;
          }
          store.dispatch(
            api.util.updateQueryData('getActivities', undefined, (draft) => {
              draft.push(newActivity);
            })
          );
        });

        stompClient.subscribe('/topic/gate-activities/delete', (message) => {
          const deletedActivityId = parseInt(message.body);
          if (isNaN(deletedActivityId)) {
            console.error('Failed to parse gate-activities/delete message: invalid activity ID');
            return;
          }
          store.dispatch(
            api.util.updateQueryData('getActivities', undefined, (draft) => {
              return draft.filter(
                (activity) => activity.id !== deletedActivityId
              );
            })
          );
        });

        stompClient.subscribe('/topic/uplinks', (message) => {
          store.dispatch(uplinkReceived(message.body));
        });

        stompClient.subscribe('/topic/health', (message) => {
          handleHealthMessage(message.body, store.dispatch);
        });

        store.dispatch(
          api.endpoints.getHealth.initiate(undefined, {
            subscribe: false,
          })
        ).then(({ data }) => {
          if (data) {
            const statuses = Object.entries(data).map(([id, dto]) => ({
              version: dto.version,
              senseGateId: Number(id),
              freeFallStatus: dto.freeFallStatus !== undefined ? dto.freeFallStatus : dto.shockStatus,
              batteryStatus: dto.batteryStatus,
              voltageMv: dto.voltageMv,
            }));
            if (statuses.length > 0) {
              store.dispatch(healthReceived({ statuses }));
            }
          }
        }).catch(() => {});
      },
      onStompError: (frame) => {
        console.error('STOMP error:', frame.headers?.message || frame.body);
        scheduleReconnect(store);
      },
      onWebSocketClose: () => {
        clientActive = false;
        scheduleReconnect(store);
      },
    });

    stompClient.activate();
  }

  function disconnect() {
    intentionalDisconnect = true;
    if (reconnectTimer) {
      clearTimeout(reconnectTimer);
      reconnectTimer = null;
    }
    if (stompClient) {
      stompClient.deactivate();
      stompClient = null;
    }
  }

  const middleware = (store) => (next) => (action) => {
    const result = next(action);

    if (
      (action.type === 'auth/setUser' ||
        action.type === 'auth/initialize/fulfilled') &&
      !clientActive
    ) {
      connect(store);
    } else if (action.type === 'auth/clearAuth') {
      disconnect();
    }

    return result;
  };

  return middleware;
}

export default createWsMiddleware();
