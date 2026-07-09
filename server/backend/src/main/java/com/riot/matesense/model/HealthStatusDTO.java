package com.riot.matesense.model;

import com.riot.matesense.enums.BatteryStatus;
import com.riot.matesense.enums.FreeFallStatus;

public class HealthStatusDTO {
    private int version;
    private int senseGateId;
    private FreeFallStatus freeFallStatus;
    private BatteryStatus batteryStatus;
    private int voltageMv;

    // Leerer Konstruktor (wichtig für JSON-Frameworks wie Jackson)
    public HealthStatusDTO() {
    }

    // Komfort-Konstruktor für den Formatter
    public HealthStatusDTO(int version, int senseGateId, FreeFallStatus freeFallStatus, BatteryStatus batteryStatus, int voltageMv) {
        this.version = version;
        this.senseGateId = senseGateId;
        this.freeFallStatus = freeFallStatus;
        this.batteryStatus = batteryStatus;
        this.voltageMv = voltageMv;
    }

    // Getter und Setter
    public int getVersion() { return version; }
    public void setVersion(int version) { this.version = version; }

    public int getSenseGateId() { return senseGateId; }
    public void setSenseGateId(int senseGateId) { this.senseGateId = senseGateId; }

    public FreeFallStatus getFreeFallStatus() { return freeFallStatus; }
    public void setFreeFallStatus(FreeFallStatus freeFallStatus) { this.freeFallStatus = freeFallStatus; }

    public BatteryStatus getBatteryStatus() { return batteryStatus; }
    public void setBatteryStatus(BatteryStatus batteryStatus) { this.batteryStatus = batteryStatus; }

    public int getVoltageMv() { return voltageMv; }
    public void setVoltageMv(int voltageMv) { this.voltageMv = voltageMv; }
}
