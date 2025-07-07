package com.riot.matesense.service;

import com.riot.matesense.config.DownPayload;
import com.riot.matesense.config.MqttProperties;
import com.riot.matesense.mqtt.TTNMqttPublisher;
import org.springframework.stereotype.Service;

import java.time.Instant;
import java.util.Arrays;
import java.util.Base64;
import java.util.List;
import java.util.Map;

//small example for some downlinks
//TODO: need to connect downlinks with request from the frontend
//TODO need to clean up a bit and connect with node team to
@Service
public class DownlinkService {

    private final TTNMqttPublisher mqttPublisher;
    private final CborConverter cborConverter;
    private final MqttProperties mqttProperties;
    private final JobTableService jobTableService;

    public DownlinkService(TTNMqttPublisher mqttPublisher, CborConverter cborConverter, MqttProperties mqttProperties, JobTableService jobTableService) {
        this.mqttPublisher = mqttPublisher;
        this.cborConverter = cborConverter;
        this.mqttProperties = mqttProperties;
    }

    public void sendDownlinkToDevice(DownPayload payloadData) {
        try {
            // === 1. Soll-Status Payload vorbereiten ===
            List<List<Integer>> sollStatusList = payloadData.getStatuses().stream()
                    .map(statusEntry -> Arrays.asList(
                            statusEntry.get(0), // GateID
                            statusEntry.get(1)  // Soll-Status
                    ))
                    .toList();

            List<Object> sollStatusPayload = Arrays.asList(
                    1,                          // Message Type für Soll-Status
                    payloadData.getTimestamp(), // Globaler Timestamp
                    sollStatusList              // Soll-Status Einträge
            );

            byte[] sollCbor = cborConverter.toCbor(sollStatusPayload);
            String sollBase64 = Base64.getEncoder().encodeToString(sollCbor);

            String sollJson = String.format("""
      {
        "downlinks": [
          {
            "f_port": 15,
            "frm_payload":"%s",
            "priority": "NORMAL"
          }
        ]
      }
      """, sollBase64);

            System.out.println("Soll-Status Downlink: " + sollJson);
            mqttPublisher.publishDownlink(sollJson.getBytes(), mqttProperties.getPublishTopic());

            // === 2. Jobtable Payload vorbereiten ===
            List<List<Integer>> jobTableList = payloadData.getStatuses().stream()
                    .map(statusEntry -> Arrays.asList(
                            statusEntry.get(0), // GateID
                            statusEntry.get(2)  // Prio
                    ))
                    .toList();

            List<Object> jobTablePayload = Arrays.asList(
                    2,              // Message Type für Jobtable
                    jobTableList    // Jobtable Einträge
            );

            byte[] jobTableCbor = cborConverter.toCbor(jobTablePayload);
            String jobTableBase64 = Base64.getEncoder().encodeToString(jobTableCbor);

            String jobTableJson = String.format("""
      {
        "downlinks": [
          {
            "f_port": 15,
            "frm_payload":"%s",
            "priority": "NORMAL"
          }
        ]
      }
      """, jobTableBase64);

            System.out.println("Jobtable Downlink: " + jobTableJson);
            mqttPublisher.publishDownlink(jobTableJson.getBytes(), mqttProperties.getPublishTopic());

        } catch (Exception e) {
            System.err.println("Fehler beim Senden: " + e.getMessage());
        }
    }
}