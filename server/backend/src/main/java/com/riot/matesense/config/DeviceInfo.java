package com.riot.matesense.config;

import lombok.Getter;

@Getter
public class DeviceInfo {
    public enum Type { GATE, MATE }

    private final String name;
    private final Type type;

    public DeviceInfo(String name, Type type) {
        this.name = name;
        this.type = type;
    }

    @Override
    public String toString() {
        return name;
    }
}
