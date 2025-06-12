package com.riot.matesense.enums;

public enum MsgType {
    SET_STATE(1),   // target Status Table (BLE)

    IST_STATE(2);   // Is Status Table (BLE)


    private final int code;

    MsgType(int code) {
        this.code = code;
    }

    public int getCode(Status status) {
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
