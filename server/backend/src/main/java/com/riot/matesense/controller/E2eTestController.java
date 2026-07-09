package com.riot.matesense.controller;

import com.riot.matesense.enums.StateConfirmation;
import com.riot.matesense.enums.BatteryStatus;
import com.riot.matesense.enums.ShockStatus;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.mqtt.MqttMessageHandler;
import com.riot.matesense.service.GateService;
import com.riot.matesense.service.HealthStatusService;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.context.annotation.Profile;
import org.springframework.http.ResponseEntity;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/e2e")
@Profile("e2e")
public class E2eTestController {

    private final MqttMessageHandler mqttMessageHandler;
    private final GateService gateService;
    private final ObjectMapper objectMapper;
    private final SimpMessagingTemplate messagingTemplate;
    private final HealthStatusService healthStatusService;

    public E2eTestController(MqttMessageHandler mqttMessageHandler, GateService gateService, ObjectMapper objectMapper, SimpMessagingTemplate messagingTemplate, HealthStatusService healthStatusService) {
        this.mqttMessageHandler = mqttMessageHandler;
        this.gateService = gateService;
        this.objectMapper = objectMapper;
        this.messagingTemplate = messagingTemplate;
        this.healthStatusService = healthStatusService;
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

    @PostMapping("/simulate-health")
    public ResponseEntity<Void> simulateHealth(@RequestBody HealthRequest request) throws Exception {
        for (HealthStatusEntry entry : request.statuses) {
            healthStatusService.updateHealth(
                entry.senseGateId,
                BatteryStatus.valueOf(entry.batteryStatus),
                ShockStatus.valueOf(entry.shockStatus),
                entry.voltageMv,
                entry.version
            );
        }
        String json = objectMapper.writeValueAsString(
                new HealthPayload(request.statuses)
        );
        messagingTemplate.convertAndSend("/topic/health", json);
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

    public static class HealthRequest {
        public List<HealthStatusEntry> statuses;
    }

    public static class HealthStatusEntry {
        public int version;
        public int senseGateId;
        public String shockStatus;
        public String batteryStatus;
        public int voltageMv;
    }

    static class HealthPayload {
        public int messageType = 5;
        public List<HealthStatusEntry> statuses;

        HealthPayload(List<HealthStatusEntry> statuses) {
            this.statuses = statuses;
        }
    }
}
