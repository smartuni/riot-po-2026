package com.riot.matesense.enums;

public enum MsgType {
    SET_STATE(0),   // target Status Table (BLE) for them
    IST_STATE(1),   // Is Status Table (BLE) for us
    SEEN_TABLE_STATE(2), //Sensemate Status Table n*m Table for us
    JOB_TABLE(3),   // Seen table for the Pilots user for them
    DUMMY_STATE(4); // dummy state for compatibility with GateController

    private final int code;

    MsgType(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    public static MsgType fromCode(int code) {
        for (MsgType type : values()) {
            if (type.code == code) {
                return type;
            }
        }
        return null;
    }
}
