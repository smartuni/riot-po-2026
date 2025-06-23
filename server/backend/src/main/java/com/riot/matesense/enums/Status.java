package com.riot.matesense.enums;

public enum Status {
    OPEN, CLOSED, UNKNOWN, NONE;

    public static Status fromCode(int code) {
        return switch (code) {
            case 0 -> OPEN;
            case 1 -> CLOSED;
            case 2 -> UNKNOWN;
            default -> null;
        };
    }
}
