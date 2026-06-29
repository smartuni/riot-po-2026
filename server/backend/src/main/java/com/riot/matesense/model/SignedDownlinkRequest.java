package com.riot.matesense.model;

import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class SignedDownlinkRequest {
    private String deviceId;
    private int gateNum;
    private int targetState;
}
