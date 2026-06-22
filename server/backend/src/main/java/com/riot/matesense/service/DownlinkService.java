package com.riot.matesense.service;

import com.fasterxml.jackson.dataformat.cbor.CBORSimpleValue;
import com.riot.matesense.config.DownPayload;
import com.riot.matesense.config.MqttProperties;
import com.riot.matesense.mqtt.TTNMqttPublisher;
import com.riot.matesense.registry.DeviceRegistry;
import org.springframework.stereotype.Service;

import java.util.*;

import static com.riot.matesense.enums.RecordType.GATE_COMMAND;

@Service
public class DownlinkService {

    private static final int MAX_COMMAND_DATA_SIZE = 28;
    private static final int CMAC_TAG_SIZE = 16;
    private static final int HEADER_SIZE = 7;
    private static final int TOTAL_PAYLOAD_SIZE = HEADER_SIZE + MAX_COMMAND_DATA_SIZE + CMAC_TAG_SIZE;

    private final TTNMqttPublisher mqttPublisher;
    private final CborConverter cborConverter;
    private final MqttProperties mqttProperties;
    private final DeviceRegistry deviceRegistry;
    private final CmacService cmacService;

    public DownlinkService(TTNMqttPublisher mqttPublisher, CborConverter cborConverter,
                           MqttProperties mqttProperties, DeviceRegistry deviceRegistry,
                           CmacService cmacService) {
        this.mqttPublisher = mqttPublisher;
        this.cborConverter = cborConverter;
        this.mqttProperties = mqttProperties;
        this.deviceRegistry = deviceRegistry;
        this.cmacService = cmacService;
    }

    public void sendDownlinkToDevice(DownPayload payloadData) {
        try {
            List<String> allDevices = new ArrayList<>();
            allDevices.addAll(deviceRegistry.getAllGateDevices());
            allDevices.addAll(deviceRegistry.getAllMateDevices());

            List<List<Integer>> sollStatusList = payloadData.getStatuses().stream()
                    .map(statusEntry -> Arrays.asList(statusEntry.get(0), statusEntry.get(1)))
                    .toList();
            for (List<Integer> gateStatePair: sollStatusList){
                byte version = 0x01;
                byte message_type = 0x01;
                byte record_type = (byte)GATE_COMMAND.getCode();
                byte[] writerId = { 0x12, 0x12, 0x12, 0x12 };
                Long sequence = 12345678L;
                long msSinceEpoch = System.currentTimeMillis();
                int hlc_phy =  (int)(msSinceEpoch / 1000);
                int hlc_log =  (int)(msSinceEpoch % 1000);

                byte device_type_gate = 0x00;
                int gate_num = gateStatePair.get(0);

                byte[] target_gate_id = { 0x00, 0x00, device_type_gate, (byte)gate_num};
                int target_state = gateStatePair.get(1);

                List<Object> sollStatusPayload = Arrays.asList(
                        version,
                        message_type,
                        record_type, writerId, sequence, hlc_phy,
                        hlc_log, target_gate_id, target_state
                );

                String sollJson = encodePayloadToBase64Json(sollStatusPayload);

                for (String gateDevice : deviceRegistry.getAllGateDevices()) {
                    String topic = mqttProperties.buildDeviceDownlinkTopic(gateDevice);
                    mqttPublisher.publishDownlink(sollJson.getBytes(), topic);
                }
            }
        } catch (Exception e) {
            System.err.println("Fehler beim Downlink-Senden: " + e.getMessage());
        }
    }

    public byte[] buildSignedDownlink(byte[] appMacKey, long hlcTimestamp, int commandId, byte[] commandData) {
        if (commandData == null) {
            commandData = new byte[0];
        }
        if (commandData.length > MAX_COMMAND_DATA_SIZE) {
            throw new IllegalArgumentException(
                    "Command data exceeds " + MAX_COMMAND_DATA_SIZE + " byte limit: " + commandData.length);
        }

        byte[] payload = new byte[TOTAL_PAYLOAD_SIZE];
        int offset = 0;

        payload[offset++] = (byte) 0x01;

        payload[offset++] = (byte) ((hlcTimestamp >> 24) & 0xFF);
        payload[offset++] = (byte) ((hlcTimestamp >> 16) & 0xFF);
        payload[offset++] = (byte) ((hlcTimestamp >> 8) & 0xFF);
        payload[offset++] = (byte) (hlcTimestamp & 0xFF);

        payload[offset++] = (byte) ((commandId >> 8) & 0xFF);
        payload[offset++] = (byte) (commandId & 0xFF);

        System.arraycopy(commandData, 0, payload, offset, commandData.length);
        offset += MAX_COMMAND_DATA_SIZE;

        byte[] dataForMac = Arrays.copyOfRange(payload, 0, HEADER_SIZE + MAX_COMMAND_DATA_SIZE);
        byte[] cmacTag = cmacService.computeCmac(appMacKey, dataForMac);

        System.arraycopy(cmacTag, 0, payload, offset, CMAC_TAG_SIZE);

        return payload;
    }

    public String sendSignedDownlink(String deviceId, Long gateId, int commandId, byte[] commandData) {
        byte[] appMacKey = deviceRegistry.getAppMacKey(deviceId);
        if (appMacKey == null) {
            throw new IllegalStateException("No AppMACKey provisioned for device: " + deviceId);
        }

        long hlcTimestamp = System.currentTimeMillis() / 1000;

        byte[] signedPayload = buildSignedDownlink(appMacKey, hlcTimestamp, commandId, commandData);

        if (signedPayload.length > 51) {
            throw new IllegalStateException("Signed payload exceeds 51 byte limit: " + signedPayload.length);
        }

        String base64Payload = Base64.getEncoder().encodeToString(signedPayload);
        String ttnJson = buildTtnJsonPayload(base64Payload);

        String topic = mqttProperties.buildDeviceDownlinkTopic(deviceId);
        mqttPublisher.publishDownlink(ttnJson.getBytes(), topic);

        return base64Payload;
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

    private String encodePayloadToBase64Json(List<Object> payload) throws Exception {
        byte[] cbor = cborConverter.toCbor(payload);
        if (cbor.length > 255) {
            throw new IllegalArgumentException("CBOR-Payload überschreitet 255-Byte-Limit: " + cbor.length + " Bytes");
        }

        String base64 = Base64.getEncoder().encodeToString(cbor);
        return String.format("""
    {
      "downlinks": [
        {
          "f_port": 15,
          "frm_payload":"%s",
          "priority": "NORMAL"
        }
      ]
    }
    """, base64);
    }
}
