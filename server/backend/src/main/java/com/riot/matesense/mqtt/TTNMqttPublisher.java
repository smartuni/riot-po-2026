package com.riot.matesense.mqtt;

import com.riot.matesense.config.MqttProperties;
import jakarta.annotation.PostConstruct;
import org.eclipse.paho.client.mqttv3.*;
import org.springframework.stereotype.Component;
@Component
public class TTNMqttPublisher {

    private final MqttProperties mqttProperties;
    private MqttClient client;

    public TTNMqttPublisher(MqttProperties mqttProperties) {
        this.mqttProperties = mqttProperties;
    }

    @PostConstruct
    public void init() {
        try {
            client = new MqttClient(mqttProperties.getBroker(), mqttProperties.getClientId() + "-publisher");
            MqttConnectOptions options = new MqttConnectOptions();
            options.setUserName(mqttProperties.getUsername());
            options.setPassword(mqttProperties.getPassword().toCharArray());
            client.connect(options);
            System.out.println("MQTT Publisher verbunden");
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public void publishDownlink(byte[] payload, String topic) {
        try {
            if (!client.isConnected()) {
                client.reconnect();
            }

            MqttMessage message = new MqttMessage(payload);
            message.setQos(1); // optional
            client.publish(topic, message);

            System.out.println("CBOR-Payload gesendet an Topic: " + topic);
        } catch (MqttException e) {
            System.err.println("Fehler beim Senden: " + e.getMessage());
        }
    }
}
