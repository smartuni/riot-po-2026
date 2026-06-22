package com.riot.matesense.controller;

import com.riot.matesense.config.DownPayload;
import com.riot.matesense.model.SignedDownlinkRequest;
import com.riot.matesense.model.SignedDownlinkResponse;
import com.riot.matesense.registry.DeviceRegistry;
import com.riot.matesense.service.DownlinkService;
//import jakarta.validation.Valid;
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

        byte[] commandData = request.getCommandData();
        if (commandData == null && request.getCommandDataHex() != null) {
            commandData = HexFormat.of().parseHex(request.getCommandDataHex());
        }
        if (commandData == null) {
            commandData = new byte[0];
        }

        if (commandData.length > 30) {
            return ResponseEntity.badRequest()
                    .body(SignedDownlinkResponse.error(deviceId,
                            "Command data exceeds 30 byte limit: " + commandData.length));
        }

        try {
            Long gateId = Long.parseLong(deviceId.replaceAll("\\D+", ""));
            String base64Payload = downlinkService.sendSignedDownlink(deviceId, gateId,
                    request.getCommandId(), commandData);

            byte[] rawPayload = Base64.getDecoder().decode(base64Payload);
            long hlcTimestamp = ((long)(rawPayload[1] & 0xFF) << 24)
                              | ((rawPayload[2] & 0xFF) << 16)
                              | ((rawPayload[3] & 0xFF) << 8)
                              |  (rawPayload[4] & 0xFF);
            SignedDownlinkResponse response = SignedDownlinkResponse.ok(
                    deviceId, request.getCommandId(), hlcTimestamp,
                    rawPayload.length, HexFormat.of().formatHex(rawPayload));

            return ResponseEntity.ok(response);
        } catch (IllegalArgumentException | IllegalStateException e) {
            return ResponseEntity.badRequest()
                    .body(SignedDownlinkResponse.error(deviceId, e.getMessage()));
        }
    }
}
