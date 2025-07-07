package com.riot.matesense.mqtt;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.enums.Status;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.Gate;
import com.riot.matesense.service.GateService;

import java.sql.Timestamp;
import java.sql.Time;

import org.springframework.stereotype.Component;

@Component
public class MqttMessageHandler {

    private final ObjectMapper mapper = new ObjectMapper();
    private final GateService gateService;

    public MqttMessageHandler(GateService gateService) {
        this.gateService = gateService;
    }

    public void msgHandlerUplinks(String decodedJson) {
        try {
            JsonNode root = mapper.readTree(decodedJson);

            if (!root.has("messageType") || !root.has("statuses")) {
                System.err.println("Ungültiges Format: " + decodedJson);
                return;
            }

            int typeCode = root.get("messageType").asInt();
            JsonNode payload = root.get("statuses");

            MsgType type = MsgType.fromCode(typeCode);
            if (type == null) {
                System.out.println("Unbekannter Nachrichtentyp: " + typeCode);
                return;
            }

            System.out.println("Verarbeiteter Nachrichtentyp: " + type + " mit Code: " + type.getCode());

            switch (type) {
                case IST_STATE -> {
                    for (JsonNode statusNode : root.get("statuses")) {
                        long gateId = statusNode.get("gateId").asLong();
                        int statusCode = statusNode.get("status").asInt();
                        Timestamp timestamp = new Timestamp(statusNode.get("timestamp").asLong());    // GateTime
                        Status status = Status.fromCode(statusCode);

                        try {
                            //update Existing Gate
                            GateEntity existingGate = gateService.getGateEntityById(gateId);
                            int confidence = existingGate.getConfidence();
                            // existingGate.setStatus(status);

                            gateService.changeGateStatus(gateId, statusCode, 1);
                            System.out.println("Gate wird aktualisiert: ID=" + gateId + ", Neuer Status=" + status);
                        } catch (GateNotFoundException e) {
                            //add new Gate
                            // GateEntity newGate = new GateEntity(); //Need to be changed
                            GateEntity newGate = new GateEntity(status, timestamp, 93.044, 51.222, "HAW", "none", 100, "none", 3  ); //Need to be changed


                            gateService.addGateFromGUI(newGate);
                            System.out.println("Gate wird neu erstellt: ID=" + gateId + "Status." + status);
                        }
                    }
                }
                // Can be used for confidence calculator
                case SEEN_TABLE_STATE -> {
                    for (JsonNode statusNode : payload) {
                        long gateId = statusNode.get("gateId").asLong();        // GateID
                        long gateTime = statusNode.get("gateTime").asLong();    // GateTime
                        int statusCode = statusNode.get("status").asInt();      // Status
                        int senseMateId = statusNode.get("senseMateId").asInt(); // SenseMateID

                        Status status = Status.fromCode(statusCode);
                        gateService.changeGateStatus(gateId, statusCode, 2);

                        System.out.println("SeenTable-Eintrag -> GateID: " + gateId +
                                ", GateTime: " + gateTime +
                                ", Status: " + status +
                                ", SenseMateID: " + senseMateId);

                    }
                }

                default -> System.out.println("Unhandled type: " + type);
            }

        } catch (JsonProcessingException e) {
            System.err.println("JSON-Parsing-Fehler: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("Fehler in msgHandler: " + e.getMessage());
        }
    }
}
