package com.riot.matesense.enums;

public enum BatteryStatus {
    CHARGING(0x00),
    DISCHARGING(0x01),
    LOW_BATTERY(0x02),
    UNKNOWN(-1); // Fallback für unerwartete Werte

    private final int code;

    BatteryStatus(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    // Wandelt den Byte-Wert direkt in das passende Enum um
    public static BatteryStatus fromCode(int code) {
        for (BatteryStatus status : values()) {
            if (status.code == code) {
                return status;
            }
        }
        return UNKNOWN;
    }
}
