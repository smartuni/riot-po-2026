package com.riot.matesense.service;

import com.riot.matesense.enums.BatteryStatus;
import com.riot.matesense.enums.FreeFallStatus;
import com.riot.matesense.model.HealthStatusDTO;
import org.springframework.stereotype.Service;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * In-memory store for the latest health status per SenseGate.
 * Health data is ephemeral (push-only from MQTT) — this service holds
 * the last-known state so the frontend can fetch it on page load
 * instead of waiting for the next WS broadcast.
 */
@Service
public class HealthStatusService {

    private final Map<Integer, HealthStatusDTO> store = new ConcurrentHashMap<>();

    public void updateHealth(int senseGateId, BatteryStatus battery, FreeFallStatus freeFall, int voltageMv, int version) {
        HealthStatusDTO existing = store.get(senseGateId);
        BatteryStatus mergedBattery = (existing != null && battery == BatteryStatus.UNKNOWN && existing.getBatteryStatus() != BatteryStatus.UNKNOWN)
                ? existing.getBatteryStatus() : battery;
        FreeFallStatus mergedFreeFall = (existing != null && freeFall == FreeFallStatus.UNKNOWN && existing.getFreeFallStatus() != FreeFallStatus.UNKNOWN)
                ? existing.getFreeFallStatus() : freeFall;
        int mergedVoltage = (existing != null && voltageMv == 0 && existing.getVoltageMv() != 0)
                ? existing.getVoltageMv() : voltageMv;
        int mergedVersion = version != 0 ? version : (existing != null ? existing.getVersion() : 1);

        store.put(senseGateId, new HealthStatusDTO(mergedVersion, senseGateId, mergedFreeFall, mergedBattery, mergedVoltage));
    }

    public Map<Integer, HealthStatusDTO> getAll() {
        return Map.copyOf(store);
    }

    public HealthStatusDTO get(int senseGateId) {
        return store.get(senseGateId);
    }

    public void clear() {
        store.clear();
    }
}
