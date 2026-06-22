package com.riot.matesense.enums;

public enum ShockStatus {
    NO_SHOCK(0x00),
    SHOCK_DETECTED(0x01),
    UNKNOWN(-1); // Fallback für unerwartete Werte

    private final int code;

    ShockStatus(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    // Wandelt den Byte-Wert direkt in das passende Enum um
    public static ShockStatus fromCode(int code) {
        for (ShockStatus status : values()) {
            if (status.code == code) {
                return status;
            }
        }
        return UNKNOWN;
    }
}
