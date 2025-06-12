package com.riot.matesense.service;
import com.riot.matesense.config.MqttProperties;
import com.riot.matesense.mqtt.TTNMqttPublisher;
import org.springframework.stereotype.Service;

import java.util.Arrays;
import java.util.List;

@Service
public class DownlinkService {

    private final TTNMqttPublisher mqttPublisher;
    private final CborConverter cborConverter;

    private final MqttProperties mqttProperties;

    public DownlinkService(TTNMqttPublisher mqttPublisher, CborConverter cborConverter, MqttProperties mqttProperties) {
        this.mqttPublisher = mqttPublisher;
        this.cborConverter = cborConverter;
        this.mqttProperties = mqttProperties;
    }

    public void sendeDownlink() {
        try {
            // Struktur entsprechend deinem Beispiel
            List<Object> downlinkPayload = Arrays.asList(
                    1,
                    247,
                    Arrays.asList(
                            Arrays.asList(187, 0),
                            Arrays.asList(69, 1)
                    )
            );

            byte[] cborPayload = cborConverter.toCbor(downlinkPayload);

            String topic = mqttProperties.getPublishTopic();// anpassen!
            mqttPublisher.publishDownlink(cborPayload, topic);
        } catch (Exception e) {
            System.err.println("Fehler bei der CBOR-Konvertierung oder dem Senden: " + e.getMessage());
        }
    }
}
