package com.riot.matesense.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.cbor.CBORFactory;
import com.riot.matesense.enums.BatteryStatus;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.enums.RecordType;
import com.riot.matesense.enums.ShockStatus;
import com.riot.matesense.model.HealthStatusDTO;
import org.springframework.stereotype.Service;

import java.nio.ByteBuffer;
import java.util.*;

import static com.riot.matesense.enums.MsgType.IST_STATE;
import static com.riot.matesense.enums.MsgType.SEEN_TABLE_STATE;
import static com.riot.matesense.enums.RecordType.GATE_OBSERVATION;
import static com.riot.matesense.enums.RecordType.GATE_REPORT;

@Service
public class JsonFormatter {

    private final ObjectMapper jsonMapper = new ObjectMapper();
    private final ObjectMapper cborMapper = new ObjectMapper(new CBORFactory());

    // ========================= DTOs =========================

    static class StatusEntry {
        public int gateId;
        public int status;
        public int timestamp;

        public StatusEntry() {}

        public StatusEntry(int gateId, int status, int timestamp) {
            this.gateId = gateId;
            this.status = status;
            this.timestamp = timestamp;
        }
    }

    static class SeenTableEntry {
        public int gateId;
        public int gateTime;
        public int status;
        public int senseMateId;

        public SeenTableEntry() {}

        public SeenTableEntry(int gateId, int gateTime, int status, int senseMateId) {
            this.gateId = gateId;
            this.gateTime = gateTime;
            this.status = status;
            this.senseMateId = senseMateId;
        }
    }

    static class Message {
        public int messageType;
        public List<?> statuses;

        public Message() {}

        public Message(int messageType, List<?> statuses) {
            this.messageType = messageType;
            this.statuses = statuses;
        }
    }

    // ========================= Methoden =========================

    public String toJsonFormat(List<Object> rawData) throws Exception {
        if (rawData == null || rawData.size() < 2) {
            throw new IllegalArgumentException("rawData ist unvollständig oder null");
        }

        // 1. Nur die absoluten Basis-Header auslesen, die JEDE Nachricht hat
        int version = ((Number) rawData.get(0)).intValue();
        int messageType = ((Number) rawData.get(1)).intValue();

        // ================= NEW: HEALTH STATUS BLOCK (Typ 5) =================
        // Wir prüfen hier nur auf die harte 5, um Enum-Fehler auszuschließen
        if (messageType == 5) {
            int senseGateId = ((Number) rawData.get(2)).intValue();
            int eventHeader = ((Number) rawData.get(3)).intValue();
            int eventBody = ((Number) rawData.get(4)).intValue();

            ShockStatus shockStatus = ShockStatus.UNKNOWN;
            BatteryStatus batteryStatus = BatteryStatus.UNKNOWN;
            int voltageMv = 0;

            switch (eventHeader) {
                case 0x00 -> { // BATTERY_CHARGING
                    batteryStatus = BatteryStatus.fromCode(0);
                    voltageMv = eventBody;
                }
                case 0x01 -> { // BATTERY_DISCHARGING
                    batteryStatus = BatteryStatus.fromCode(1);
                    voltageMv = eventBody;
                }
                case 0x02 -> { // BATTERY_LOW
                    batteryStatus = BatteryStatus.fromCode(2);
                    voltageMv = eventBody;
                }
                case 0x03 -> { // SHOCK_STATUS
                    shockStatus = ShockStatus.fromCode(eventBody);
                }
                case 0x04 -> {
                    System.out.println("FREE_FALL Event empfangen (noch nicht voll implementiert)");
                }
                default -> System.err.println("Unbekannter Health Event Header: " + eventHeader);
            }

            HealthStatusDTO healthDTO = new HealthStatusDTO(version, senseGateId, shockStatus, batteryStatus, voltageMv);
            Message message = new Message(messageType, List.of(healthDTO));

            // Verlässt die Methode sofort für Typ 5. Der alte Code darunter wird nie erreicht!
            return jsonMapper.writerWithDefaultPrettyPrinter().writeValueAsString(message);
        }

        RecordType recordType = RecordType.fromCode((int)rawData.get(2));
        byte[] writerId= (byte[])rawData.get(3);
        int hlc_phy = (int)rawData.get(5);
        int hlc_log = (int)rawData.get(6);
        //===== Header -----^^^^^

        // TODO: add proper support for HLC instead fo this custom conversion onto the old int-timestamp
        int timestamp = hlc_phy * 1000 + hlc_log;
        /*
        int cnt = 0;
        for (Object o: rawData) {
            System.out.println("========");
            System.out.println(cnt++);
            System.out.println(o.getClass());
            System.out.println(o);
            System.out.println("========");
        }
        */

        if (GATE_REPORT.equals(recordType)) {
            int gateid = writerId[3];
            int gateState = (int)rawData.get(7);
            StatusEntry se = new StatusEntry(gateid, gateState, timestamp);
            Message message = new Message(IST_STATE.getCode(), List.of(se));
            return jsonMapper.writerWithDefaultPrettyPrinter().writeValueAsString(message);
        } else if (GATE_OBSERVATION.equals(recordType)) {
            List<SeenTableEntry> seenTableList = new ArrayList<>();
            byte[] gateId= (byte[])rawData.get(7);
            //TODO: add native support of new node_id type
            int gateid = gateId[3];
            int gateState = (int)rawData.get(8);
            int mateid = writerId[3];

            seenTableList.add(new SeenTableEntry(gateid, timestamp, gateState, mateid));
            Message message = new Message(SEEN_TABLE_STATE.getCode(), seenTableList);
            return jsonMapper.writerWithDefaultPrettyPrinter().writeValueAsString(message);
        }else {
            throw new IllegalArgumentException("Unknown RecordType: " + recordType);
        }
    }

    public List<Object> fromJsonFormat(String jsonString) throws Exception {
        JsonNode root = jsonMapper.readTree(jsonString);
        int messageType = root.get("messageType").asInt();

        List<Object> result = new ArrayList<>();
        result.add(messageType);

        List<List<Integer>> entries = new ArrayList<>();

        if (messageType == 1) { // IST_STATE
            for (JsonNode statusNode : root.get("statuses")) {
                int gateId = statusNode.get("gateId").asInt();
                int status = statusNode.get("status").asInt();
                int timestamp = statusNode.get("timestamp").asInt();
                entries.add(Arrays.asList(gateId, status, timestamp));
            }
        } else if (messageType == 2) { // SEEN_TABLE_STATE
            for (JsonNode statusNode : root.get("statuses")) {
                int gateId = statusNode.get("gateId").asInt();
                int gateTime = statusNode.get("gateTime").asInt();
                int status = statusNode.get("status").asInt();
                int senseMateId = statusNode.get("senseMateId").asInt();
                entries.add(Arrays.asList(gateId,status,gateTime, senseMateId));
            }
        } else if (messageType == 5) { // HEALTH_MONITORING
            // Falls das Inbound-System (z.B. für Tests) auch die Rückrichtung de-serialisieren muss:
            for (JsonNode statusNode : root.get("statuses")) {
                int senseGateId = statusNode.get("senseGateId").asInt();
                int voltageMv = statusNode.get("voltageMv").asInt();
                // Enums/Strings als Code mappen wenn nötig
                entries.add(Arrays.asList(senseGateId, voltageMv));
            }
        } else {
            throw new IllegalArgumentException("Unbekannter MessageType: " + messageType);
        }

        result.add(entries);
        return result;
    }

    public byte[] toCborBytes(List<Object> data) throws Exception {
        return cborMapper.writeValueAsBytes(data);
    }
}
