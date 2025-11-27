package com.riot.matesense.enums;

public enum RecordType {
    GATE_REPORT(0),
    GATE_OBSERVATION(1),
    GATE_COMMAND(3),
    GATE_JOB(4);
    private final int code;

    RecordType(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    public static RecordType fromCode(int code) {
        for (RecordType type : values()) {
            if (type.code == code) {
                return type;
            }
        }
        return null;
    }
}
