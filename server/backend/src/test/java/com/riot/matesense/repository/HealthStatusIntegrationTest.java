package com.riot.matesense.repository;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.riot.matesense.enums.BatteryStatus;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.enums.FreeFallStatus;
import com.riot.matesense.service.JsonFormatter;
import org.junit.jupiter.api.Test;

import java.util.Arrays;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class HealthStatusIntegrationTest {

    private final JsonFormatter jsonFormatter = new JsonFormatter();
    private final ObjectMapper mapper = new ObjectMapper();

    @Test
    void testHealthMonitoring_FreeFallEvent() throws Exception {
        // 1. Vorbereitung der Testdaten laut neuem Ticket (Event-basiert)
        int version = 1;
        int messageType = MsgType.HEALTH_MONITORING.getCode(); // 5
        int senseGateId = 1;      // rawData.get(2) -> Kommt jetzt als Zahl/Integer an
        int eventHeader = 0x03;   // rawData.get(3) -> FREE_FALL_STATUS
        int eventBody = 0x02;     // rawData.get(4) -> Free Fall detected

        List<Object> mockRawData = Arrays.asList(
                version,
                messageType,
                senseGateId,
                eventHeader,
                eventBody
        );

        // 2. Ausführung
        String jsonResult = jsonFormatter.toJsonFormat(mockRawData);
        System.out.println("Generiertes JSON für Free Fall Event:\n" + jsonResult);

        // 3. Überprüfung
        JsonNode root = mapper.readTree(jsonResult);
        assertEquals(5, root.get("messageType").asInt());
        assertTrue(root.has("statuses"));

        JsonNode healthNode = root.get("statuses").get(0);
        assertEquals(1, healthNode.get("version").asInt());
        assertEquals(1, healthNode.get("senseGateId").asInt());
        assertEquals("FREE_FALL_DETECTED", healthNode.get("freeFallStatus").asText());
    }

    @Test
    void testHealthMonitoring_BatteryEvent() throws Exception {
        // 1. Vorbereitung der Testdaten laut neuem Ticket (Event-basiert)
        int version = 1;
        int messageType = MsgType.HEALTH_MONITORING.getCode(); // 5
        int senseGateId = 1;      // rawData.get(2)
        int eventHeader = 0x02;   // rawData.get(3) -> BATTERY_LOW
        int eventBody = 3700;     // rawData.get(4) -> 3700 mV (Spannung steht im Body!)

        List<Object> mockRawData = Arrays.asList(
                version,
                messageType,
                senseGateId,
                eventHeader,
                eventBody
        );

        // 2. Ausführung
        String jsonResult = jsonFormatter.toJsonFormat(mockRawData);
        System.out.println("Generiertes JSON für Batterie-Event:\n" + jsonResult);

        // 3. Überprüfung
        JsonNode root = mapper.readTree(jsonResult);
        assertEquals(5, root.get("messageType").asInt());
        assertTrue(root.has("statuses"));

        JsonNode healthNode = root.get("statuses").get(0);
        assertEquals(1, healthNode.get("version").asInt());
        assertEquals(1, healthNode.get("senseGateId").asInt());
        assertEquals("LOW_BATTERY", healthNode.get("batteryStatus").asText());
        assertEquals(3700, healthNode.get("voltageMv").asInt());
    }
}
