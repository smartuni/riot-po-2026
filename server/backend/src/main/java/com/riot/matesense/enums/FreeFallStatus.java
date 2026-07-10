package com.riot.matesense.enums;

public enum FreeFallStatus {
    NO_FALL(0x00),
    FREE_FALL_DETECTED(0x02),
    UNKNOWN(-1); // Fallback für unerwartete Werte

    private final int code;

    FreeFallStatus(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    // Wandelt den Byte-Wert direkt in das passende Enum um
    public static FreeFallStatus fromCode(int code) {
        for (FreeFallStatus status : values()) {
            if (status.code == code) {
                return status;
            }
        }
        return UNKNOWN;
    }
}
