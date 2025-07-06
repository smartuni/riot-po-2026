package com.riot.matesense.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.cbor.CBORFactory;
import org.springframework.stereotype.Service;

import java.io.FileOutputStream;
import java.io.IOException;
import java.util.*;
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
        int messageType = (int) rawData.get(0);
        List<List<Integer>> entries = (List<List<Integer>>) rawData.get(1);

        if (messageType == 1) { // IST_STATE
            List<StatusEntry> statusList = new ArrayList<>();
            for (List<Integer> entry : entries) {
                statusList.add(new StatusEntry(entry.get(0), entry.get(1), entry.get(2)));
            }
            Message message = new Message(messageType, statusList);
            return jsonMapper.writerWithDefaultPrettyPrinter().writeValueAsString(message);
        } else if (messageType == 3) { // SEEN_TABLE_STATE
            List<SeenTableEntry> seenTableList = new ArrayList<>();
            for (List<Integer> entry : entries) {
                seenTableList.add(new SeenTableEntry(entry.get(0), entry.get(1), entry.get(2), entry.get(3)));
            }
            Message message = new Message(messageType, seenTableList);
            return jsonMapper.writerWithDefaultPrettyPrinter().writeValueAsString(message);
        } else {
            throw new IllegalArgumentException("Unbekannter MessageType: " + messageType);
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
        } else if (messageType == 3) { // SEEN_TABLE_STATE
            for (JsonNode statusNode : root.get("statuses")) {
                int gateId = statusNode.get("gateId").asInt();
                int gateTime = statusNode.get("gateTime").asInt();
                int status = statusNode.get("status").asInt();
                int senseMateId = statusNode.get("senseMateId").asInt();
                entries.add(Arrays.asList(gateId, gateTime, status, senseMateId));
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
