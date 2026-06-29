package com.riot.matesense.service;

import com.riot.matesense.config.DownPayload;
import com.riot.matesense.config.MqttProperties;
import com.riot.matesense.mqtt.TTNMqttPublisher;
import com.riot.matesense.registry.DeviceRegistry;
import org.springframework.stereotype.Service;

import java.util.Arrays;
import java.util.Base64;
import java.util.List;

import static com.riot.matesense.enums.RecordType.GATE_COMMAND;

@Service
public class DownlinkService {

    private static final int CMAC_TAG_SIZE = 16;

    private final TTNMqttPublisher mqttPublisher;
    private final MqttProperties mqttProperties;
    private final DeviceRegistry deviceRegistry;
    private final CmacService cmacService;

    public DownlinkService(TTNMqttPublisher mqttPublisher,
                           MqttProperties mqttProperties, DeviceRegistry deviceRegistry,
                           CmacService cmacService) {
        this.mqttPublisher = mqttPublisher;
        this.mqttProperties = mqttProperties;
        this.deviceRegistry = deviceRegistry;
        this.cmacService = cmacService;
    }

    public void sendDownlinkToDevice(DownPayload payloadData) {
        try {
            List<List<Integer>> sollStatusList = payloadData.getStatuses().stream()
                    .map(statusEntry -> Arrays.asList(statusEntry.get(0), statusEntry.get(1)))
                    .toList();
            for (List<Integer> gateStatePair : sollStatusList) {
                int gateNum = gateStatePair.get(0);
                int targetState = gateStatePair.get(1);
                String base64Payload = buildSignedGateCommandCbor(gateNum, targetState, null);

                for (String gateDevice : deviceRegistry.getAllGateDevices()) {
                    String topic = mqttProperties.buildDeviceDownlinkTopic(gateDevice);
                    String ttnJson = buildTtnJsonPayload(base64Payload);
                    mqttPublisher.publishDownlink(ttnJson.getBytes(), topic);
                }
            }
        } catch (Exception e) {
            System.err.println("Fehler beim Downlink-Senden: " + e.getMessage());
        }
    }

    public String sendSignedDownlink(String deviceId, int gateNum, int targetState) {
        byte[] appMacKey = deviceRegistry.getAppMacKey(deviceId);
        if (appMacKey == null) {
            throw new IllegalStateException("No AppMACKey provisioned for device: " + deviceId);
        }

        String base64Payload = buildSignedGateCommandCbor(gateNum, targetState, appMacKey);
        String ttnJson = buildTtnJsonPayload(base64Payload);

        String topic = mqttProperties.buildDeviceDownlinkTopic(deviceId);
        mqttPublisher.publishDownlink(ttnJson.getBytes(), topic);

        return base64Payload;
    }

    String buildSignedGateCommandCbor(int gateNum, int targetState, byte[] appMacKey) {
        GateCommandRecord record = toGateCommandRecord(gateNum, targetState);

        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);

        byte[] signature;
        if (appMacKey != null) {
            byte[] cmac = cmacService.computeCmac(appMacKey, unsignedCbor);
            signature = Arrays.copyOf(cmac, CMAC_TAG_SIZE);
        } else {
            signature = null;
        }

        byte[] signedCbor = FirmwareCborSerializer.serialize(record, signature);

        if (signedCbor.length > 51) {
            throw new IllegalStateException("Signed CBOR downlink exceeds 51 byte limit: " + signedCbor.length);
        }

        return Base64.getEncoder().encodeToString(signedCbor);
    }

    private GateCommandRecord toGateCommandRecord(int gateNum, int targetState) {
        long nowMs = System.currentTimeMillis();
        long epochSeconds = nowMs / 1000;

        return new GateCommandRecord(
                0x01,
                0x01,
                GATE_COMMAND.getCode(),
                new byte[]{0x12, 0x12, 0x12, 0x12},
                epochSeconds,
                epochSeconds,
                0,
                new byte[]{0x00, 0x00, 0x00, (byte) gateNum},
                mapGateState(targetState)
        );
    }

    static int mapGateState(int backendState) {
        return switch (backendState) {
            case 0 -> 0;
            case 1 -> 1;
            default -> throw new IllegalArgumentException("Unsupported gate state: " + backendState);
        };
    }

    private String buildTtnJsonPayload(String base64Payload) {
        return String.format("""
                {
                  "downlinks": [
                    {
                      "f_port": 15,
                      "frm_payload": "%s",
                      "priority": "NORMAL"
                    }
                  ]
                }
                """, base64Payload);
    }
}
