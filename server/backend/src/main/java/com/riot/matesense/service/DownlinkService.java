package com.riot.matesense.service;

import com.riot.matesense.config.DownPayload;
import com.riot.matesense.config.MqttProperties;
import com.riot.matesense.mqtt.TTNMqttPublisher;
import com.riot.matesense.registry.DeviceRegistry;
import org.springframework.stereotype.Service;

import java.util.*;

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

            List<Object> sollStatusPayload = Arrays.asList(
                    0, payloadData.getTimestamp(), 2, 0, sollStatusList
            );

            String sollJson = encodePayloadToBase64Json(sollStatusPayload);
            System.out.println("Soll-Status JSON: " + sollJson);

            for (String gateDevice : deviceRegistry.getAllGateDevices()) {
                String topic = mqttProperties.buildDeviceDownlinkTopic(gateDevice);
                mqttPublisher.publishDownlink(sollJson.getBytes(), topic);
                System.out.println("Soll-Status gesendet an: " + topic);
            }
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