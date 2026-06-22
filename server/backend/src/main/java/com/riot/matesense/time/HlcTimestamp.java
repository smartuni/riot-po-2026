package com.riot.matesense.time;

public class HlcTimestamp {

    private final long physical;
    private final long logical;

    public HlcTimestamp(long physical, long logical) {
        this.physical = physical;
        this.logical = logical;
    }

    public long getPhysical() {
        return physical;
    }

    public long getLogical() {
        return logical;
    }

    @Override
    public String toString() {
        return "(" + physical + ", " + logical + ")";
    }
}