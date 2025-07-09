package com.riot.matesense.mqtt;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.riot.matesense.entity.GateActivityEntity;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.enums.Status;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.Gate;
import com.riot.matesense.registry.DeviceRegistry;
import com.riot.matesense.service.GateActivityService;
import com.riot.matesense.service.GateService;

import java.sql.Timestamp;
import java.sql.Time;

import org.springframework.stereotype.Component;

@Component
public class MqttMessageHandler {

    private final ObjectMapper mapper = new ObjectMapper();
    private final GateService gateService;
    GateActivityService gateActivityService;
    private final DeviceRegistry deviceRegistry;

    public MqttMessageHandler(GateService gateService, GateActivityService gateActivityService, DeviceRegistry deviceRegistry) {
        this.gateService = gateService;
        this.gateActivityService = gateActivityService;
        this.deviceRegistry = deviceRegistry;
    }

    public void msgHandlerUplinks(String decodedJson,String deviceName) {
        // Sicherheit: Nur sensegate-* oder sensemate-* zulassen
        if (!deviceName.startsWith("sensegate-") && !deviceName.startsWith("sensemate-")) {
            System.err.println(" Nicht erlaubtes Gerät sendet Uplink: " + deviceName);
            return;
        }
        deviceRegistry.registerDevice(deviceName);
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
                        //can be better - maybe but this back in the enum Status
                        Status status;
                            switch (statusCode) {
                                case 0:
                                    status = Status.CLOSED;
                                break;
                                case 1:
                                    status = Status.OPENED;
                                break;
                                case 2:
                                    status = Status.UNKNOWN;
                                break;
                                default:
                                    status = Status.NONE;
                                break;
                            }

                        try {
                            //update Existing Gate
                            GateEntity existingGate = gateService.getGateEntityById(gateId);
                            if(Math.abs(timestamp.getTime() - existingGate.getLastTimeStamp().getTime()) < 500){
                                System.out.println("Timestamp are Equal");
                                System.out.println("GateID:" + gateId + "Timestamp" + timestamp.getTime());
                                continue;
                            }
//                            int confidence = existingGate.getConfidence();
                            // existingGate.setStatus(status);


                            gateService.changeGateStatus(gateId,status, 1);
                            gateActivityService.addGateActivity(new GateActivityEntity(timestamp, gateId, status.toString(), "Gate has changed to status " + status.toString(), null));
                            System.out.println("Gate wird aktualisiert: ID=" + gateId + ", Neuer Status=" + status);
                        } catch (GateNotFoundException e) {
                            //add new Gate

                            // GateEntity newGate = new GateEntity(); //Need to be changed
                            GateEntity newGate = new GateEntity(gateId,status, timestamp, 93.044, 51.222, "HAW", "none", 100, "none", 3  ); //Need to be changed
                            gateService.addGateFromGUI(newGate);
                            System.out.println("Gate wird neu erstellt: ID=" + gateId + "Status." + status);
                            System.out.println("GateID:" + gateId + "Timestamp" + timestamp.getTime());

                        }
                    }
                }
                // Can be used for confidence calculator
                case SEEN_TABLE_STATE -> {
                    for (JsonNode statusNode : payload) {
                        long gateId = statusNode.get("gateId").asLong();        // GateID
                        int statusCode = statusNode.get("status").asInt();      // Status
                        Timestamp gateTime = new Timestamp(statusNode.get("gateTime").asLong());    // GateTime
                        int senseMateId = statusNode.get("senseMateId").asInt(); // SenseMateID

                        Status status;
                        switch (statusCode) {
                            case 0:
                                status = Status.CLOSED;
                                break;
                            case 1:
                                status = Status.OPENED;
                                break;
                            case 2:
                                status = Status.UNKNOWN;
                                break;
                            default:
                                status = Status.NONE;
                                break;
                        }

                        GateEntity existingGate = gateService.getGateEntityById(gateId);

                        int confidence = existingGate.getConfidence();

                        gateService.changeGateStatus(gateId, status, 2);
                        System.out.println("SeenTable-Eintrag -> GateID: " + gateId +
                                ", Status: " + status +
                                ", GateTime: " + gateTime +
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
