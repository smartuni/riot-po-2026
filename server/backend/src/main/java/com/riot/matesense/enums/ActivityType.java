package com.riot.matesense.enums;

public enum ActivityType {
    SENSOR_NEW(0),              // a previously unknown sensor was added
    SENSOR_VALUE_CHANGED(1),    // The sensor reported a new state
    SENSOR_VALUE_KEEPALIVE(2),  // The sensor confirmed the current state with a new uplink
    SENSEMATE_WORKER_REPORT(3), // An worker observation was reported through a sensemate
    TARGET_STATE_REQUEST(4);    // A worker requested a gate shall be put into a given target state (currently only doable via dashboard UI)

    private final int code;

    ActivityType(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    public static ActivityType fromCode(int code) {
        for (ActivityType type : values()) {
            if (type.code == code) {
                return type;
            }
        }
        return null;
    }
}