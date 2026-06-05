package com.riot.matesense.config;

import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class DeviceInfo {
    public enum Type { GATE, MATE }

    private final String name;
    private final Type type;

    private byte[] appMacKey;
    private int lastSeqTx;

    public DeviceInfo(String name, Type type) {
        this.name = name;
        this.type = type;
    }

    @Override
    public String toString() {
        return name;
    }
}
