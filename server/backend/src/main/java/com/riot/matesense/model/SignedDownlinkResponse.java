package com.riot.matesense.model;

import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class SignedDownlinkResponse {
    private boolean success;
    private String message;
    private String deviceId;
    private int commandId;
    private long hlcTimestamp;
    private int payloadSizeBytes;
    private String payloadHex;
    private String error;

    public SignedDownlinkResponse() {}

    public static SignedDownlinkResponse ok(String deviceId, int commandId, long hlcTimestamp,
                                             int payloadSize, String payloadHex) {
        SignedDownlinkResponse r = new SignedDownlinkResponse();
        r.success = true;
        r.message = "Signed downlink sent successfully";
        r.deviceId = deviceId;
        r.commandId = commandId;
        r.hlcTimestamp = hlcTimestamp;
        r.payloadSizeBytes = payloadSize;
        r.payloadHex = payloadHex;
        return r;
    }

    public static SignedDownlinkResponse error(String deviceId, String error) {
        SignedDownlinkResponse r = new SignedDownlinkResponse();
        r.success = false;
        r.deviceId = deviceId;
        r.error = error;
        r.message = "Failed to send signed downlink: " + error;
        return r;
    }
}
