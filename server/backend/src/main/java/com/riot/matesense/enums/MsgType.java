package com.riot.matesense.enums;
public enum MsgType {
    SET_STATE(0),   // target Status Table (BLE)
    IST_STATE(1),   // Is Status Table (BLE)
    JOB_TABLE(2),   // Seen table for the Pilots user
    SEEN_TABLE_STATE(3); //Sensemate Status Table n*m Table

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
