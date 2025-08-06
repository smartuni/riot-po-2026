package com.riot.matesense.enums;

public enum Status {
    CLOSED, OPENED, UNKNOWN, NONE;

    public static Status fromCode(int statusCode)
    {
        switch (statusCode) 
        {
            case 0:
                return Status.CLOSED;
            case 1:
                return Status.OPENED;
            case 2:
                return Status.UNKNOWN;
            default:
                return Status.NONE;
        }
    }
}
