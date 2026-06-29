package com.riot.matesense.controller;

import com.riot.matesense.config.DownPayload;
import com.riot.matesense.model.SignedDownlinkRequest;
import com.riot.matesense.model.SignedDownlinkResponse;
import com.riot.matesense.registry.DeviceRegistry;
import com.riot.matesense.service.DownlinkService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.Map;
import java.util.Base64;
import java.util.HexFormat;

@RestController
@RequestMapping("/downlink")
public class DownlinkController {

    private final DownlinkService downlinkService;
    private final DeviceRegistry deviceRegistry;

    public DownlinkController(DownlinkService downlinkService, DeviceRegistry deviceRegistry) {
        this.downlinkService = downlinkService;
        this.deviceRegistry = deviceRegistry;
    }

    @PostMapping
    public ResponseEntity<Map<String, String>> sendDownlink(
            @RequestBody DownPayload payload) {
        downlinkService.sendDownlinkToDevice(payload);
        return ResponseEntity.ok(Map.of("message", "Downlink vorbereitet."));
    }

    @PostMapping("/signed")
    public ResponseEntity<SignedDownlinkResponse> sendSignedDownlink(
            @RequestBody SignedDownlinkRequest request) {

        String deviceId = request.getDeviceId();
        if (deviceId == null || deviceId.isBlank()) {
            return ResponseEntity.badRequest()
                    .body(SignedDownlinkResponse.error(null, "deviceId is required"));
        }

        if (!deviceRegistry.isRegistered(deviceId)) {
            return ResponseEntity.badRequest()
                    .body(SignedDownlinkResponse.error(deviceId, "Device not registered"));
        }

        byte[] appMacKey = deviceRegistry.getAppMacKey(deviceId);
        if (appMacKey == null) {
            return ResponseEntity.badRequest()
                    .body(SignedDownlinkResponse.error(deviceId, "No AppMACKey provisioned for device"));
        }

        try {
            String base64Payload = downlinkService.sendSignedDownlink(
                    deviceId, request.getGateNum(), request.getTargetState());

            byte[] rawPayload = Base64.getDecoder().decode(base64Payload);
            SignedDownlinkResponse response = SignedDownlinkResponse.ok(
                    deviceId, rawPayload.length, HexFormat.of().formatHex(rawPayload));

            return ResponseEntity.ok(response);
        } catch (IllegalArgumentException | IllegalStateException e) {
            return ResponseEntity.badRequest()
                    .body(SignedDownlinkResponse.error(deviceId, e.getMessage()));
        }
    }
}
