package com.riot.matesense.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.riot.matesense.entity.GateActivityEntity;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.enums.Status;
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

    public List<Gate> getAllGates() {
        List<GateEntity> gates = gateRepository.findAll();
        List<Gate> customGates = new ArrayList<>();
        gates.forEach(e -> {
            Gate gate = new Gate(e.getId(), e.getDeviceId(), e.getLastTimeStamp(), e.getStatus(),
                    e.getLatitude(), e.getLongitude(), e.getLocation(), e.getSensorConfidence(),
                    e.getWorkerConfidence(), e.getRequestedStatus(), e.getConfidence(), e.getPendingJob(), e.getPriority());
            customGates.add(gate);
        });
        return customGates;
    }

    public String addGate(GateEntity gate) throws GateAlreadyExistingException {
        gateRepository.save(gate);
        messagingTemplate.convertAndSend("/topic/gate-activities/add", gate);
        return gate.toString();
    }

    public String addGateFromGUI(GateEntity gate) throws GateAlreadyExistingException {
        gate.setPriority(3);
        gate.setLastTimeStamp(new Timestamp(System.currentTimeMillis()));
        //TODO: if confidence calc is done remove this
        gate.setConfidence("100");
        gateRepository.save(gate);
        //TODO: IS THIS THE RIGHT WAY?
        messagingTemplate.convertAndSend("/topic/gates/add", gate);
        // Notify all clients about the new gate
        return gate.toString();
    }


    public void removeGate(GateEntity gate){
        gateRepository.delete(gate);
    }

    public void removeGateById(Long id) throws GateNotFoundException {
        GateEntity gate = gateRepository.getById(id);
        if (gate == null) {
            throw new GateNotFoundException(id);
        }
        gateRepository.delete(gate);
        messagingTemplate.convertAndSend("/topic/gates/delete", id);
    }

    public void updateGate(GateEntity gate) {
        GateEntity gateEntity = new GateEntity();
        if (gateRepository.getById(gate.getId()) == null) {
            gateRepository.save(gate);
        } else {
            gateEntity.setRequestedStatus(gate.getRequestedStatus());
            gateEntity.setLastTimeStamp(gate.getLastTimeStamp());
            gateEntity.setDeviceId(gate.getDeviceId());
            gateEntity.setStatus(gate.getStatus());
            gateEntity.setSensorConfidence(gate.getSensorConfidence());
            gateEntity.setWorkerConfidence(gate.getWorkerConfidence());
            gateEntity.setLocation(gate.getLocation());
            gateRepository.save(gateEntity);
        }
    }
    public GateEntity getGateEntityById(Long id) throws GateNotFoundException {
        return gateRepository.findById(Math.toIntExact(id)).orElseThrow(() -> new GateNotFoundException(id));
    }

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

    public void updatePriority(Long gateId, int newPriority) {
        GateEntity gateEntity = gateRepository.getById(gateId);
        gateEntity.setPriority(newPriority);
        gateRepository.save(gateEntity);
    }
}
