package com.riot.matesense.registry;
import org.springframework.stereotype.Component;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

@Component
public class DeviceRegistry {

    private final Map<Integer, String> gateDevices = new ConcurrentHashMap<>();
    private final Map<Integer, String> mateDevices = new ConcurrentHashMap<>();

    //register devices that was writte in the uplink
    private void register(Map<Integer, String> deviceMap, String deviceName, String typeLabel) {
        int id = parseId(deviceName);
        if (id == -1) return;

        if (!deviceMap.containsKey(id)) {
            deviceMap.put(id, deviceName);
            System.out.println("Neues " + typeLabel + "-Gerät registriert: " + deviceName);
        }
        System.out.println(typeLabel + "-Gerät registriert: " + deviceName);
    }

    //Find out which sense it is
    public void registerDevice(String deviceName) {
        if (deviceName.startsWith("sensegate-")) {
            register(gateDevices, deviceName, "Gate");
        } else if (deviceName.startsWith("sensemate-")) {
            register(mateDevices, deviceName, "Mate");
        } else {
            System.err.println("Ungültiger Gerätename: " + deviceName);
        }
    }


    public Set<String> getAllGateDevices() {
        return new HashSet<>(gateDevices.values());
    }

    public Set<String> getAllMateDevices() {
        return new HashSet<>(mateDevices.values());
    }

    public Set<String> getAllDevices() {
        Set<String> all = new HashSet<>();
        all.addAll(gateDevices.values());
        all.addAll(mateDevices.values());
        return all;
    }

    public int getGateDeviceCount() {
        return gateDevices.size();
    }

    public int getMateDeviceCount() {
        return mateDevices.size();
    }


    private int parseId(String deviceName) {
        try {
            String[] parts = deviceName.split("-");
            if (parts.length != 2) throw new IllegalArgumentException("Ungültiges Format");
            return Integer.parseInt(parts[1]);
        } catch (Exception e) {
            System.err.println("Fehler beim Parsen der ID aus '" + deviceName + "': " + e.getMessage());
            return -1; // oder andere Fehlerbehandlung
        }
    }

}
