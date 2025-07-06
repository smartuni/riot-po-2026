package com.riot.matesense.mqtt;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.riot.matesense.config.MqttProperties;
import com.riot.matesense.service.Base64ToList;
import com.riot.matesense.service.JsonFormatter;
import jakarta.annotation.PostConstruct;
import org.eclipse.paho.client.mqttv3.*;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;

@Component
public class TTNMqttListener {

    private final MqttProperties mqttProperties;
    private final ObjectMapper mapper = new ObjectMapper();
    private final Base64ToList converter;
    private final JsonFormatter jsonFormatter;
    private final MqttMessageHandler mqttMessageHandler;

    @Autowired
    public TTNMqttListener(
            MqttProperties mqttProperties,
            Base64ToList converter,
            JsonFormatter jsonFormatter,
            MqttMessageHandler mqttMessageHandler) {
        this.mqttProperties = mqttProperties;
        this.converter = converter;
        this.jsonFormatter = jsonFormatter;
        this.mqttMessageHandler = mqttMessageHandler;
    }

    @PostConstruct
    public void init() {
        try {
            MqttClient client = new MqttClient(mqttProperties.getBroker(), mqttProperties.getClientId());
            MqttConnectOptions options = new MqttConnectOptions();
            options.setUserName(mqttProperties.getUsername());
            options.setPassword(mqttProperties.getPassword().toCharArray());

            client.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    System.out.println("Verbindung verloren: " + cause.getMessage());
                }

                @Override
                public void messageArrived(String topic, MqttMessage message) {
                    try {
                        String payloadStr = new String(message.getPayload());
                        JsonNode root = mapper.readTree(payloadStr);
                        JsonNode uplink = root.path("uplink_message");

                        if (!uplink.isMissingNode()) {
                            JsonNode frmPayloadNode = uplink.path("frm_payload");
                            if (!frmPayloadNode.isMissingNode()) {
                                String frmPayloadBase64 = frmPayloadNode.asText();
                                System.out.println("frm_payload erkannt: " + frmPayloadBase64);

                                List<Object> decodedList = converter.decodeBase64ToList(frmPayloadBase64);
                                String formattedJson = jsonFormatter.toJsonFormat(decodedList);

                                System.out.println("Dekodiertes JSON: " + formattedJson);

                                mqttMessageHandler.msgHandlerUplinks(formattedJson);
                            }
                        }
                    } catch (Exception e) {
                        System.err.println("Fehler beim Verarbeiten der Nachricht: " + e.getMessage());
                    }
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {}
            });

            client.connect(options);
            client.subscribe(mqttProperties.getSubscribeTopic());
            System.out.println("MQTT-Client gestartet, warte auf Nachrichten...");
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }
}
