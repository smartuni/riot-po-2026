package com.riot.matesense.registry;

import com.riot.matesense.config.DeviceInfo;
import org.springframework.stereotype.Component;

import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

@Component
public class DeviceRegistry {

    private final Map<String, DeviceInfo> devices = new ConcurrentHashMap<>();

    public void registerDevice(String deviceName) {
        DeviceInfo.Type type;

        if (deviceName.startsWith("sensegate-")) {
            type = DeviceInfo.Type.GATE;
        } else if (deviceName.startsWith("sensemate-")) {
            type = DeviceInfo.Type.MATE;
        } else {
            System.err.println(" Ungültiger Gerätename: " + deviceName);
            return;
        }

        devices.putIfAbsent(deviceName, new DeviceInfo(deviceName, type));
        System.out.println("Gerät registriert: " + deviceName + " (" + type + ")");
    }

    public boolean removeDevice(String deviceName) {
        return devices.remove(deviceName) != null;
    }

    public Set<String> getAllGateDevices() {
        return devices.values().stream()
                .filter(d -> d.getType() == DeviceInfo.Type.GATE)
                .map(DeviceInfo::getName)
                .collect(Collectors.toSet());
    }

    public Set<String> getAllMateDevices() {
        return devices.values().stream()
                .filter(d -> d.getType() == DeviceInfo.Type.MATE)
                .map(DeviceInfo::getName)
                .collect(Collectors.toSet());
    }

    public Set<String> getAllDevices() {
        return devices.keySet();
    }

    public int getGateDeviceCount() {
        return (int) devices.values().stream()
                .filter(d -> d.getType() == DeviceInfo.Type.GATE)
                .count();
    }

    public int getMateDeviceCount() {
        return (int) devices.values().stream()
                .filter(d -> d.getType() == DeviceInfo.Type.MATE)
                .count();
    }

    public boolean isRegistered(String deviceName) {
        return devices.containsKey(deviceName);
    }
}
