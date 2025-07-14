package com.riot.matesense.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.riot.matesense.entity.GateActivityEntity;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.enums.Status;
import com.riot.matesense.service.ConfidenceCalculator;
import com.riot.matesense.enums.ConfidenceQuality;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.Gate;
import com.riot.matesense.model.GateForDownlink;
import com.riot.matesense.repository.GateRepository;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import javax.tools.Diagnostic;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

@Service
public class GateService {

    @Autowired
    GateRepository gateRepository;
    private SimpMessagingTemplate messagingTemplate;
    MsgType msgType;
    ConfidenceCalculator calculator;

    
    public GateService(GateRepository gateRepository, SimpMessagingTemplate messagingTemplate) {
        this.gateRepository = gateRepository;
        this.messagingTemplate = messagingTemplate;
        this.calculator = new ConfidenceCalculator();
    }

    /**
     * a Method to get all GateEntities from the DB
     * @return a list with all the gates
     */
    public List<Gate> getAllGates() {
        List<GateEntity> gates = gateRepository.findAll();
        List<Gate> customGates = new ArrayList<>();
        gates.forEach(e -> {
            System.out.println("getAll");
            // changeConfidence(e, (int)(Math.random() * 100));
            Gate gate = new Gate(e.getId(), e.getDeviceId(), e.getLastTimeStamp(), e.getStatus(),
                    e.getLatitude(), e.getLongitude(), e.getLocation(), 
                    e.getWorkerConfidence(), e.getSensorConfidence(), e.getRequestedStatus(), e.getConfidence(), e.getQuality(), e.getPendingJob(), e.getPriority());
            customGates.add(gate);
        });
        return customGates;
    }

    /**
     * a Method to add a Gate to be DB
     * @param gate to be added
     * @return the gate as a String
     * @throws GateAlreadyExistingException
     */
    public String addGate(GateEntity gate) throws GateAlreadyExistingException {
        gateRepository.save(gate);
        messagingTemplate.convertAndSend("/topic/gate-activities/add", gate);
        return gate.toString();
    }


    /*public void removeGate(GateEntity gate){
        gateRepository.delete(gate);
    }*/

    /**
     * a method to remove a gate by the given gateID
     * @param gateId of the gate that should be removed
     * @throws GateNotFoundException
     */
    public void removeGateById(Long gateId) throws GateNotFoundException {
        GateEntity gate = gateRepository.getById(gateId);
        if (gate == null) {
            throw new GateNotFoundException(gateId);
        }
        gateRepository.delete(gate);
        messagingTemplate.convertAndSend("/topic/gates/delete", gateId);
    }


    /**
     * a method to update the gate
     * @param gate that should be updated
     * @param reportType of the msg
     */
    public void updateGate(GateEntity gate, MsgType reportType) {
        //Hole das Gate
        //GateEntity existingGate = gateRepository.findById(Math.toIntExact(gate.getId())).orElse(null);
       //Existiert das Gate schon
//        if (existingGate == null) {
//            System.out.println("Gate nicht gefunden. Füge neues Gate hinzu.");
//            gateRepository.save(gate);
//            return;
//        }
        gate.setRequestedStatus(gate.getRequestedStatus());
        gate.setLastTimeStamp(gate.getLastTimeStamp());
        gate.setDeviceId(gate.getDeviceId());
        gate.setStatus(gate.getStatus());
        //Set confidence
        gate.setSensorConfidence(gate.getSensorConfidence());
        gate.setWorkerConfidence(gate.getWorkerConfidence());
        //get location from gates
        gate.setLocation(gate.getLocation());
        //Set Arraylist for worker and Gates
        gate.setGateStatusArray(gate.getGateStatusArray());
        gate.setWorkerStatusArray(gate.getWorkerStatusArray());

        calculator.changeConfidence(gate, gate.getConfidence(), reportType);
        //gate.setConfidence(gate.getConfidence());
        gateRepository.save(gate);

    }

    /**
     * a method to check if a gate exists by its id
     * @param id of the gate
     * @return true if it exists
     */
    public boolean existsGateById(Long id) {
        return gateRepository.existsById(Math.toIntExact(id));
    }

    /**
     * return a gate by its id
     * @param id of the gate
     * @return the gate if it exists
     * @throws GateNotFoundException
     */
    public GateEntity getGateEntityById(Long id) throws GateNotFoundException {
        return gateRepository.findById(Math.toIntExact(id)).orElseThrow(() -> new GateNotFoundException(id));
    }

    /**
     * a method to get a gate by its id
     * @param id of the gate
     * @return the gate
     * @throws GateNotFoundException
     */
    public Gate getGateById(Long id) throws GateNotFoundException {
        GateEntity gate = gateRepository.findById(Math.toIntExact(id)).orElseThrow(() -> new GateNotFoundException(id));
        return new Gate(gate.getId(), gate.getDeviceId(), gate.getLastTimeStamp(), gate.getStatus(),
                gate.getLatitude(), gate.getLongitude(), gate.getLocation(), gate.getWorkerConfidence(),
                gate.getSensorConfidence(), gate.getRequestedStatus(), gate.getConfidence(), gate.getQuality(), gate.getPendingJob(), gate.getPriority());
    }

    //TODO need to be checked - here can be a bug - need to be tested to!!

//    public Gate getGateById(Long id) {
//        GateEntity gate = gateRepository.getById(id);
//        return new Gate(gate.getId(), gate.getDeviceId(), gate.getLastTimeStamp(), gate.getStatus(),
//                gate.getLatitude(), gate.getLongitude(), gate.getLocation(), gate.getWorkerConfidence(),
//                gate.getSensorConfidence(), gate.getRequestedStatus(), gate.getConfidence(), gate.getQuality());
//    }

    /**
     * a method to change the requested status of a gate
     * @param gateId of the gate
     * @param targetStatus for the gate
     */
    public void requestGateStatusChange(Long gateId, String targetStatus) {
        GateEntity gate = gateRepository.getById(gateId);
        System.out.println("Current Status: " + gate.getStatus());
        System.out.println("Requested Status: " + targetStatus);
        System.out.println("ID: " + gate.getId());

        // Ziel-Status aus requestedStatus ableiten
        String tmp;
        switch (targetStatus) {
            case "REQUESTED_OPEN" -> tmp = "OPENED";
            case "REQUESTED_CLOSE" -> tmp = "CLOSED";
            case "REQUESTED_NONE" -> tmp = "NONE";
            default -> tmp = targetStatus;
        }

        // 1. Pending-Job **immer setzen**, basierend auf targetStatus
        switch (targetStatus) {
            case "REQUESTED_OPEN" -> gate.setPendingJob("PENDING_OPEN");
            case "REQUESTED_CLOSE" -> gate.setPendingJob("PENDING_CLOSE");
            case "REQUESTED_NONE" -> gate.setPendingJob("PENDING_NONE");
        }

        // 2. Nur wenn tatsächlicher Status ≠ Ziel, dann requestedStatus setzen
        if (!tmp.equalsIgnoreCase(gate.getStatus().toString().strip())) {
            if (targetStatus.equals("REQUESTED_NONE") || targetStatus.equals("NONE")) {
                gate.setRequestedStatus(null);
            } else {
                gate.setRequestedStatus(targetStatus);
            }
        }

        gate.setLastTimeStamp(new Timestamp(System.currentTimeMillis()));
        gateRepository.save(gate);
    }

    public void changeGateStatus(Long gateId, Status status, MsgType reportType) {
        GateEntity gate = gateRepository.getById(gateId);
        int confidence = gate.getConfidence();

        // Ziel-Status aus requestedStatus ableiten
        // switch (targetStatus) {
        //     case "REQUESTED_OPEN" -> tmp = "OPENED";
        //     case "REQUESTED_CLOSE" -> tmp = "CLOSED";
        //     case "REQUESTED_NONE" -> tmp = "NONE";
        //     default -> tmp = targetStatus;
        // }

        // 1. Pending-Job **immer setzen**, basierend auf targetStatus
        // switch (targetStatus) {
        //     case "REQUESTED_OPEN" -> gate.setPendingJob("PENDING_OPEN");
        //     case "REQUESTED_CLOSE" -> gate.setPendingJob("PENDING_CLOSE");
        //     case "REQUESTED_NONE" -> gate.setPendingJob("PENDING_NONE");
        // }

        // 2. Nur wenn tatsächlicher Status ≠ Ziel, dann requestedStatus setzen
        // if (!tmp.equalsIgnoreCase(gate.getStatus().toString().strip())) {
            // if (targetStatus.equals("REQUESTED_NONE") || targetStatus.equals("NONE")) {
            //     gate.setStatus(null);
            // } else {

//
//        if (status == Status.OPENED) {
//            if ("PENDING_OPEN".equals(gate.getPendingJob())) {
//                gate.setPendingJob("None");
//            }
//        } else if(status == Status.CLOSED){
//            if ("PENDING_CLOSE".equals(gate.getPendingJob())) {
//                gate.setPendingJob("None");
//            }
//        }

        gate.setStatus(status);
        //dont be surprised if pending job didn't change after the first status change! It need to be 100% confidence
        calculator.changeConfidence(gate, confidence, reportType);
        messagingTemplate.convertAndSend("/topic/gates/updates", gate);
            // }
        // }

        gate.setLastTimeStamp(new Timestamp(System.currentTimeMillis()));
        gateRepository.save(gate);
    }

    /**
     * a method to all the gates and format them into gatesForDownLink Entities
     * @return a List with the formatted gates
     */
    public List<GateForDownlink> getAllGatesForDownlink() {
        List<GateEntity> gates = gateRepository.findAll();
        List<GateForDownlink> customGates = new ArrayList<>();
        gates.forEach(e -> {
            int requestedStatus;
            switch (e.getRequestedStatus()) {
                case "REQUESTED_OPEN":
                    requestedStatus = 1;
                    break;
                case "REQUESTED_CLOSE":
                    requestedStatus = 0;
                    break;
                case "REQUESTED_NONE":
                    requestedStatus = 2;
                    break;
                default:
                    requestedStatus = 0; // Default to CLOSED if status is unknown
            }
            GateForDownlink gate = new GateForDownlink(Math.toIntExact(e.getId()), requestedStatus);
            customGates.add(gate);
        });
        return customGates;
    }

    /**
     * a method to update the priority of the gate
     * @param gateId of the gate that should be changed
     * @param newPriority for the gate
     */
    public void updatePriority(Long gateId, int newPriority) {
        GateEntity gateEntity = gateRepository.getById(gateId);
        gateEntity.setPriority(newPriority);
        gateRepository.save(gateEntity);
    }

    /**
     * a method to get the next highest ID for the gates so there wont be collisions
     * @return the next free id
     */
    public Long getIdForGate(){
        List<GateEntity> gates = gateRepository.findAll();
        if (gates.isEmpty()) {
            return 1L; // Start with ID 1 if no gates exist
        } else {
            Long maxId = gates.stream()
                    .map(GateEntity::getId)
                    .max(Long::compareTo)
                    .orElse(0L);
            return maxId + 1;
        }
    }

    /**
     * a method to add a basic Gate to the DB
     * @param gate that should be added to the DB
     * @return the gate as a String
     * @throws GateAlreadyExistingException
     */
    public String addGateFromGUI(GateEntity gate) throws GateAlreadyExistingException {
        if (gate.getId() == null){
            gate.setId(getIdForGate());
        }
        gate.setPriority(3);
        if (gate.getLastTimeStamp() == null) {
            gate.setLastTimeStamp(new Timestamp(System.currentTimeMillis()));
        }
        gate.setLastTimeStamp(gate.getLastTimeStamp());
        //TODO: löschen falls problematisch mit den gates von den Nodes
        gate.setRequestedStatus("REQUESTED_NONE");
        gate.setPendingJob("PENDING_NONE");
        gateRepository.save(gate);
        messagingTemplate.convertAndSend("/topic/gates/add", gate);
        // Notify all clients about the new gate
        return gate.toString();
    }
}
