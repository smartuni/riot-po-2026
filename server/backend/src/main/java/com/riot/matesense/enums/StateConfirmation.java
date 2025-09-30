package com.riot.matesense.enums;

public enum StateConfirmation {
    WORKER_CONFLICT(0),         // at least one worker reported state that is in conflict with the sensor-reported value
    UNCONFIRMED(1),             // state is based only on the sensor reading (but no conflict)
    WORKER_CONFIRMED_SINGLE(2), // the state was confirmed by one SenseMate user
    WORKER_CONFIRMED_MULTI(3),  // the state was confirmed by at least two SenseMate users
    WORKER_CONFIRMED_ALL(4);    // the state was confirmed by all known SenseMate users

    private final int code;

    StateConfirmation(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    public static StateConfirmation fromCode(int code) {
        for (StateConfirmation type : values()) {
            if (type.code == code) {
                return type;
            }
        }
        return null;
    }
}