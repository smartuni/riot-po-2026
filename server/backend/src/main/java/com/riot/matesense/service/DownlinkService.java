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
        this.jobTableService = jobTableService;
    }

    //TODO Need to create a loop to send down a publish for every enddevices - need to seperated devices
    //TODO Beetween Sensmate and GateMate ID !!
    //TODO Need to create two list for request, for Sensemate and Gate devices.
    //Creating second list next to Target_Table
    public void sendDownlinkToDevice(DownPayload payloadData) {
        try {
            // Wandlung des POJO in eine strukturierte Liste:
            List<Object> payload = Arrays.asList(
                    payloadData.getMessageType(),
                    payloadData.getTimestamp(),
                    payloadData.getStatuses() // entspricht List<List<Integer>>
            );

            byte[] cbor = cborConverter.toCbor(payload);
            String base64 = Base64.getEncoder().encodeToString(cbor);
            System.out.println("BASE64 repr: " + base64);

            String json = String.format("""
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

            System.out.println("FINAL JSON: " + json);

            //TODO Create a Job table here before a publish
            //jobTableService.(Some Object): //TODO creating tupel[2,[1,3],[2,1],...]
            mqttPublisher.publishDownlink(json.getBytes(), mqttProperties.getPublishTopic());
        } catch (Exception e) {
            System.err.println("Fehler beim Senden: " + e.getMessage());
        }
    }
}