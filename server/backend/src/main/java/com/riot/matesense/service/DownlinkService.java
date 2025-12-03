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

    private final TTNMqttPublisher mqttPublisher;
    private final CborConverter cborConverter;
    private final MqttProperties mqttProperties;

    private final DeviceRegistry deviceRegistry;
    public DownlinkService(TTNMqttPublisher mqttPublisher, CborConverter cborConverter, MqttProperties mqttProperties, DeviceRegistry deviceRegistry) {
        this.mqttPublisher = mqttPublisher;
        this.cborConverter = cborConverter;
        this.mqttProperties = mqttProperties;
        this.deviceRegistry = deviceRegistry;
    }

    public void sendDownlinkToDevice(DownPayload payloadData) {
        try {
            List<String> allDevices = new ArrayList<>();
            allDevices.addAll(deviceRegistry.getAllGateDevices());
            allDevices.addAll(deviceRegistry.getAllMateDevices());

            // === Soll-Status vorbereiten ===
            List<List<Integer>> sollStatusList = payloadData.getStatuses().stream()
                    .map(statusEntry -> Arrays.asList(statusEntry.get(0), statusEntry.get(1)))
                    .toList();
            for (List<Integer> gateStatePair: sollStatusList){
                //===== HEADER vvvv
                byte version = 0x01; // fixed for now
                byte message_type = 0x01; // message type single report (fixed for now)
                byte record_type = (byte)GATE_COMMAND.getCode();
                byte[] writerId = { 0x12, 0x12, 0x12, 0x12 };
                //byte[] sequence = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, (byte)0x88};
                Long sequence = 12345678L;
                long msSinceEpoch = System.currentTimeMillis();
                int hlc_phy =  (int)(msSinceEpoch / 1000);
                int hlc_log =  (int)(msSinceEpoch % 1000);
                //===== HEADER ^^^^

                byte device_type_gate = 0x00;
                int gate_num = gateStatePair.get(0);

                //===== Gate Command vvvv
                byte[] target_gate_id = { 0x00, 0x00, device_type_gate, (byte)gate_num};
                int target_state = gateStatePair.get(1);
                //===== Gate Command ^^^^

                List<Object> sollStatusPayload = Arrays.asList(
                        //0, payloadData.getTimestamp(), 2, 0, sollStatusList
                        version,
                        message_type,
                        record_type, writerId, sequence, hlc_phy,
                        hlc_log, target_gate_id, target_state
                );

                String sollJson = encodePayloadToBase64Json(sollStatusPayload);
                System.out.println("Soll-Status JSON: " + sollJson);

                for (String gateDevice : deviceRegistry.getAllGateDevices()) {
                    String topic = mqttProperties.buildDeviceDownlinkTopic(gateDevice);
                    mqttPublisher.publishDownlink(sollJson.getBytes(), topic);
                    System.out.println("Soll-Status gesendet an: " + topic);
                }
            }
            //List<Object> sollStatusPayload = Arrays.asList(
            //        0, payloadData.getTimestamp(), 2, 0, sollStatusList
            //);

        } catch (Exception e) {
            System.err.println("Fehler beim Downlink-Senden: " + e.getMessage());
        }
    }

    private String encodePayloadToBase64Json(List<Object> payload) throws Exception {
        byte[] cbor = cborConverter.toCbor(payload);
        System.out.println("CBOR-Payload lenght: " + cbor.length);
        //need to be send to the frontend and need to be handle
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