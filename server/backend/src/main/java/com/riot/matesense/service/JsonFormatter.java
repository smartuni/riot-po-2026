//package com.riot.matesense.service;
//
//import com.fasterxml.jackson.databind.ObjectMapper;
//import org.springframework.stereotype.Service;
//
//import java.util.ArrayList;
//import java.util.List;
//
//@Service
//public class JsonFormatter {
//
//    public static class StatusEntry {
//        public int gateId;
//        public int status;
//        public int timestamp;
//
//        public StatusEntry(int gateId, int status, int timestamp) {
//            this.gateId = gateId;
//            this.status = status;
//            this.timestamp = timestamp;
//        }
//    }
//
//    public static class Message {
//        public int messageType;
//        public List<StatusEntry> statuses;
//
//        public Message(int messageType, List<StatusEntry> statuses) {
//            this.messageType = messageType;
//            this.statuses = statuses;
//        }
//    }
//
//    public String toJsonFormat(List<Object> rawData) throws Exception {
//        int messageType = (int) rawData.get(0);
//        List<List<Integer>> entries = (List<List<Integer>>) rawData.get(1);
//        List<StatusEntry> statusList = new ArrayList<>();
//
//        for (List<Integer> entry : entries) {
//            statusList.add(new StatusEntry(entry.get(0), entry.get(1), entry.get(2)));
//        }
//
//        Message message = new Message(messageType, statusList);
//        ObjectMapper mapper = new ObjectMapper();
//        return mapper.writerWithDefaultPrettyPrinter().writeValueAsString(message);
//    }
//}

package com.riot.matesense.service;

import com.fasterxml.jackson.core.type.TypeReference;
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

    // Interne DTOs
    static class StatusEntry {
        public int gateId;
        public int status;
        public int timestamp;

        public StatusEntry() {} // Für Jackson

        public StatusEntry(int gateId, int status, int timestamp) {
            this.gateId = gateId;
            this.status = status;
            this.timestamp = timestamp;
        }
    }

    static class Message {
        public int messageType;
        public List<StatusEntry> statuses;

        public Message() {}

        public Message(int messageType, List<StatusEntry> statuses) {
            this.messageType = messageType;
            this.statuses = statuses;
        }
    }

    /**
     * Konvertiert strukturierte Rohdaten (z. B. von MQTT Uplink) zu formatiertem JSON
     */
    public String toJsonFormat(List<Object> rawData) throws Exception {
        int messageType = (int) rawData.get(0);
        List<List<Integer>> entries = (List<List<Integer>>) rawData.get(1);
        List<StatusEntry> statusList = new ArrayList<>();

        for (List<Integer> entry : entries) {
            statusList.add(new StatusEntry(entry.get(0), entry.get(1), entry.get(2)));
        }

        Message message = new Message(messageType, statusList);
        return jsonMapper.writerWithDefaultPrettyPrinter().writeValueAsString(message);
    }

    /**
     * Wandelt JSON-String (mit messageType & statuses) in ursprüngliche Struktur zurück
     */
    public List<Object> fromJsonFormat(String jsonString) throws Exception {
        Message message = jsonMapper.readValue(jsonString, new TypeReference<>() {});
        List<Object> result = new ArrayList<>();
        result.add(message.messageType);

        List<List<Integer>> entries = new ArrayList<>();
        for (StatusEntry entry : message.statuses) {
            entries.add(Arrays.asList(entry.gateId, entry.status, entry.timestamp));
        }

        result.add(entries);
        return result;
    }

    /**
     * Codiert verschachtelte Datenstruktur als CBOR-Bytearray
     */
    public byte[] toCborBytes(List<Object> data) throws Exception {
        return cborMapper.writeValueAsBytes(data);
    }

    /**
     * Optional: schreibt CBOR-Bytes in Datei
     */
    public void writeCborToFile(byte[] cborBytes, String filePath) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(filePath)) {
            fos.write(cborBytes);
        }
    }
}

