package com.riot.matesense.service;

import com.riot.matesense.config.DownPayload;
import com.riot.matesense.config.MqttProperties;
import com.riot.matesense.mqtt.TTNMqttPublisher;
import com.riot.matesense.repository.GateRepository;
import org.springframework.stereotype.Service;

import java.time.Instant;
import java.util.Arrays;
import java.util.Base64;
import java.util.List;
import java.util.Map;
//TODO do timestamp for the targetTable
//small example for some downlinks
//TODO: need to connect downlinks with request from the frontend
//TODO need to clean up a bit and connect with node team to
@Service
public class DownlinkService {

    private final TTNMqttPublisher mqttPublisher;
    private final CborConverter cborConverter;
    private final MqttProperties mqttProperties;
    private final GateRepository gateRepository;

    public DownlinkService(TTNMqttPublisher mqttPublisher, CborConverter cborConverter, MqttProperties mqttProperties, GateRepository gateRepository) {
        this.mqttPublisher = mqttPublisher;
        this.cborConverter = cborConverter;
        this.mqttProperties = mqttProperties;
        this.gateRepository = gateRepository;
    }

    public void sendDownlinkToDevice(DownPayload payloadData) {
        //sensegate-*
        try {
            System.out.println("Soll-Status Downlink: " + payloadData.getStatuses());
            System.out.println("Get TimeStamp" + payloadData.getTimestamp());

            // === 1. Soll-Status Payload vorbereiten ===
            List<List<Integer>> sollStatusList = payloadData.getStatuses().stream()
                    .map(statusEntry -> Arrays.asList(
                            statusEntry.get(0), // GateID
                            statusEntry.get(1) // Soll-Status
                    ))
                    .toList();

            List<Object> sollStatusPayload = Arrays.asList(
                    0,                          // Message Type für Soll-Status
                    payloadData.getTimestamp(), // Globaler Timestamp
                    2,                          // ServerSide
                    0,                          // devicesID
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

            //sensemate-*
            System.out.println("Soll-Status Downlink: " + sollJson);
            mqttPublisher.publishDownlink(sollJson.getBytes(), mqttProperties.getPublishTopic());
            System.out.println("Soll-Status Downlink published And job Table is starting");
            // === 2. Jobtable Payload vorbereiten ===
            List<List<Integer>> jobTableList = payloadData.getStatuses().stream()
                    .map(statusEntry -> Arrays.asList(
                            statusEntry.get(0), // GateID
                            statusEntry.get(2)
                    ))
                    .toList();

            List<Object> jobTablePayload = Arrays.asList(
                    3,              // Message Type für Jobtable
                    jobTableList    // Jobtable Einträge
            );
            System.out.println("JobTable list Created");

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