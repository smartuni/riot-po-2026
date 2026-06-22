package com.riot.matesense.repository;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.riot.matesense.enums.BatteryStatus;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.enums.ShockStatus;
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
    void testHealthMonitoringDecodingFlow() throws Exception {
        // 1. Vorbereitung der Testdaten (Byte-Array-Struktur analog zum Ticket/Hardware-Absprache)
        int version = 1;
        int messageType = MsgType.HEALTH_MONITORING.getCode(); // 5
        byte[] writerId = new byte[]{0x00, 0x00, 0x00, 0x01};  // ID = 1
        int shockStatus = ShockStatus.SHOCK_DETECTED.getCode(); // 1
        int batteryStatus = BatteryStatus.LOW_BATTERY.getCode(); // 2
        int voltageMv = 3700; // 3700 mV

        // Die Liste, die dein Base64ToList-Converter an den Formatter übergeben würde
        List<Object> mockRawData = Arrays.asList(
                version,
                messageType,
                writerId,
                shockStatus,
                batteryStatus,
                voltageMv
        );

        // 2. Ausführung der Methode im JsonFormatter
        String jsonResult = jsonFormatter.toJsonFormat(mockRawData);
        System.out.println("Generiertes JSON für das Frontend:\n" + jsonResult);

        // 3. Überprüfung (Assertions)
        JsonNode root = mapper.readTree(jsonResult);

        // Stimmen die umschließenden Felder der Message-Klasse?
        assertEquals(5, root.get("messageType").asInt());
        assertTrue(root.has("statuses"));

        // Stimmen die Werte im geschachtelten HealthStatusDTO?
        JsonNode healthNode = root.get("statuses").get(0);
        assertEquals(1, healthNode.get("version").asInt());
        assertEquals(1, healthNode.get("senseGateId").asInt());
        assertEquals("SHOCK_DETECTED", healthNode.get("shockStatus").asText());
        assertEquals("LOW_BATTERY", healthNode.get("batteryStatus").asText());
        assertEquals(3700, healthNode.get("voltageMv").asInt());
    }
}
