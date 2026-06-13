package com.riot.matesense.controller;

import com.riot.matesense.enums.StateConfirmation;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.mqtt.MqttMessageHandler;
import com.riot.matesense.service.GateService;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.context.annotation.Profile;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/e2e")
@Profile("e2e")
public class E2eTestController {

    private final MqttMessageHandler mqttMessageHandler;
    private final GateService gateService;
    private final ObjectMapper objectMapper;

    public E2eTestController(MqttMessageHandler mqttMessageHandler, GateService gateService, ObjectMapper objectMapper) {
        this.mqttMessageHandler = mqttMessageHandler;
        this.gateService = gateService;
        this.objectMapper = objectMapper;
    }

    @PostMapping("/simulate-uplink")
    public ResponseEntity<Void> simulateUplink(@RequestBody UplinkRequest request) throws Exception {
        String json = objectMapper.writeValueAsString(
                new UplinkPayload(request.messageType, request.statuses)
        );
        mqttMessageHandler.msgHandlerUplinks(json, request.deviceName);
        return ResponseEntity.ok().build();
    }

    @PostMapping("/simulate-state-confirmation")
    public ResponseEntity<Void> simulateStateConfirmation(@RequestBody StateConfirmationRequest request) throws GateNotFoundException {
        gateService.changeGateStateConfirmation(request.gateId, request.state);
        return ResponseEntity.ok().build();
    }

    // --- Request DTOs ---

    public static class UplinkRequest {
        public String deviceName;
        public int messageType;
        public List<StatusEntry> statuses;
    }

    public static class StatusEntry {
        public long gateId;
        public int status;
        public long timestamp;
    }

    // --- Internal payload for JSON serialization ---

    static class UplinkPayload {
        public int messageType;
        public List<StatusEntry> statuses;

        UplinkPayload(int messageType, List<StatusEntry> statuses) {
            this.messageType = messageType;
            this.statuses = statuses;
        }
    }

    public static class StateConfirmationRequest {
        public Long gateId;
        public StateConfirmation state;
    }
}
