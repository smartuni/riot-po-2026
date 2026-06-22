package com.riot.matesense.model;

import com.riot.matesense.enums.BatteryStatus;
import com.riot.matesense.enums.ShockStatus;

public class HealthStatusDTO {
    private int version;
    private int senseGateId;
    private ShockStatus shockStatus;
    private BatteryStatus batteryStatus;
    private int voltageMv;

    // Leerer Konstruktor (wichtig für JSON-Frameworks wie Jackson)
    public HealthStatusDTO() {
    }

    // Komfort-Konstruktor für den Formatter
    public HealthStatusDTO(int version, int senseGateId, ShockStatus shockStatus, BatteryStatus batteryStatus, int voltageMv) {
        this.version = version;
        this.senseGateId = senseGateId;
        this.shockStatus = shockStatus;
        this.batteryStatus = batteryStatus;
        this.voltageMv = voltageMv;
    }

    // Getter und Setter
    public int getVersion() { return version; }
    public void setVersion(int version) { this.version = version; }

    public int getSenseGateId() { return senseGateId; }
    public void setSenseGateId(int senseGateId) { this.senseGateId = senseGateId; }

    public ShockStatus getShockStatus() { return shockStatus; }
    public void setShockStatus(ShockStatus shockStatus) { this.shockStatus = shockStatus; }

    public BatteryStatus getBatteryStatus() { return batteryStatus; }
    public void setBatteryStatus(BatteryStatus batteryStatus) { this.batteryStatus = batteryStatus; }

    public int getVoltageMv() { return voltageMv; }
    public void setVoltageMv(int voltageMv) { this.voltageMv = voltageMv; }
}
