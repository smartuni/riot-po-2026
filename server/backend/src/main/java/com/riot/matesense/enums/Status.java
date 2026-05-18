package com.riot.matesense.enums;

/**
 * A Status for the Gate
 */
public enum Status {
    OPEN, CLOSED, OUT_OF_SERVICE, NONE;

    public static Status fromCode(int statusCode)
    {
        switch (statusCode)
        {
            case 0:
                return Status.OPEN;
            case 1:
                return Status.CLOSED;
            case 2:
                return Status.OUT_OF_SERVICE;
            default:
                return Status.NONE;
        }
    }
}
