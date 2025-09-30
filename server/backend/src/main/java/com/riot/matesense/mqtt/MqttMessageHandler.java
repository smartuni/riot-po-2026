package com.riot.matesense.mqtt;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.riot.matesense.entity.GateActivityEntity;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.enums.StateConfirmation;
import com.riot.matesense.enums.Status;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.GateActivity;
import com.riot.matesense.registry.DeviceRegistry;
import com.riot.matesense.service.GateActivityService;
import com.riot.matesense.service.GateService;

import java.sql.Timestamp;
import java.util.List;

import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Component;

@Component
public class MqttMessageHandler {

    private final ObjectMapper mapper = new ObjectMapper();
    private final GateService gateService;
    GateActivityService gateActivityService;
    private final DeviceRegistry deviceRegistry;
    private SimpMessagingTemplate messagingTemplate;

    public MqttMessageHandler(GateService gateService, GateActivityService gateActivityService, DeviceRegistry deviceRegistry, SimpMessagingTemplate messagingTemplate) {
        this.gateService = gateService;
        this.gateActivityService = gateActivityService;
        this.deviceRegistry = deviceRegistry;
        this.messagingTemplate = messagingTemplate;
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

            String messagestring = "Verarbeiteter Nachrichtentyp: " + type + " mit Code: " + type.getCode();
            System.out.println("Verarbeiteter Nachrichtentyp: " + type + " mit Code: " + type.getCode());
            messagingTemplate.convertAndSend("/topic/uplinks", messagestring);
            switch (type) {
                case IST_STATE -> {
                    for (JsonNode statusNode : root.get("statuses")) {
                        long gateId = statusNode.get("gateId").asLong();
                        int statusCode = statusNode.get("status").asInt();
                        Timestamp gateTimeStamp = new Timestamp(statusNode.get("timestamp").asLong());
                        Timestamp localTimeStamp = new Timestamp(System.currentTimeMillis());
                        Status status = Status.fromCode(statusCode);

                        try {
                            GateEntity existingGate = gateService.getGateEntityById(gateId);

                            if (!existingGate.getStatus().equals(status)) {
                                // on changes mark the new state as unconfirmed
                                gateService.changeGateStateConfirmation(gateId, StateConfirmation.UNCONFIRMED);
                            }
                            gateService.changeGateStatus(gateId, status, MsgType.IST_STATE, gateTimeStamp);
                            gateActivityService.addGateActivity(new GateActivityEntity(localTimeStamp, gateTimeStamp, gateId, status.toString(), "Gate " + gateId + " has changed to status " + status.toString(), null));
                            System.out.println("Gate wird aktualisiert: ID=" + gateId + ", Neuer Status=" + status);
                        } catch (GateNotFoundException e) {
                            //add new Gate

                            // GateEntity newGate = new GateEntity(); //Need to be changed
                            GateEntity newGate = new GateEntity(gateId,status, localTimeStamp, 53.557120, 10.022826, "HAW", "REQUESTED_NONE", 0, "PENDING_NONE", 3  ); //Need to be changed
                            gateService.addGateFromGUI(newGate);
                            System.out.println("Gate wird neu erstellt: ID=" + gateId + "Status." + status);
                            System.out.println("GateID:" + gateId + "Timestamp" + localTimeStamp.getTime());
                            gateActivityService.addGateActivity(new GateActivityEntity(localTimeStamp, gateTimeStamp, gateId, status.toString(), "New Gate " + gateId + " has been added with status " + status.toString(), null));

                        }
                    }
                }
                // Can be used for confidence calculator
                case SEEN_TABLE_STATE -> {
                    for (JsonNode statusNode : payload) {
                        long gateId = statusNode.get("gateId").asLong();        // GateID
                        int statusCode = statusNode.get("status").asInt();      // Status
                        long gateTime = statusNode.get("gateTime").asLong();
                        Timestamp gateTimeStamp = new Timestamp(gateTime);
                        Timestamp localTimeStamp = new Timestamp(System.currentTimeMillis());
                        System.out.println(gateTime);
                        System.out.println(gateTimeStamp);
                        System.out.println(localTimeStamp);
                        Long senseMateId = statusNode.get("senseMateId").asLong(); // SenseMateID

                        List<GateActivity> allGateActivities = gateActivityService.getGateActivitiesByGateId(gateId);
                        Status status = Status.fromCode(statusCode);
                        GateActivityEntity ngae = new GateActivityEntity(localTimeStamp, gateTimeStamp, gateId, status.toString(), "Gate "+ gateId +" was reported as " + status.toString() + " by sensemate-" + senseMateId, senseMateId);
                        GateActivity nga = new GateActivity(ngae.getLocalTimeStamp(), ngae.getGateTimeStamp(), ngae.getGateId(), ngae.getRequestedStatus(), ngae.getMessage(), ngae.getId(), ngae.getWorkerId());
                        boolean already_exists = false;
                        for (GateActivity ega: allGateActivities) {
                            if (ega.getGateTimeStamp().equals(nga.getGateTimeStamp())) {
                                System.out.println("This seen state was already reported!");
                                already_exists = true;
                                break;
                            }
                        }

                        if (!already_exists) { //TODO: move this to the gateActivityService itself!
                            gateActivityService.addGateActivity(ngae);
                            System.out.println("SeenTable-Eintrag -> GateID: " + gateId +
                                    ", Status: " + status +
                                    ", GateTime: " + gateTime +
                                    ", SenseMateID: " + senseMateId);
                        }
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
