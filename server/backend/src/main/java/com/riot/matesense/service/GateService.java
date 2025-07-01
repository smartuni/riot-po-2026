package com.riot.matesense.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.enums.Status;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.Gate;
import com.riot.matesense.model.GateForDownlink;
import com.riot.matesense.repository.GateRepository;
import org.springframework.beans.factory.annotation.Autowired;
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
        return gate.toString();
    }


    public void removeGate(GateEntity gate){
        gateRepository.delete(gate);
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

    public Gate getGateById(Long id) {
        GateEntity gate = gateRepository.getById(id);
        return new Gate(gate.getId(), gate.getDeviceId(), gate.getLastTimeStamp(), gate.getStatus(),
                gate.getLatitude(), gate.getLongitude(), gate.getLocation(), gate.getWorkerConfidence(),
                gate.getSensorConfidence(), gate.getRequestedStatus(), gate.getConfidence(), gate.getPendingJob(), gate.getPriority());
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
