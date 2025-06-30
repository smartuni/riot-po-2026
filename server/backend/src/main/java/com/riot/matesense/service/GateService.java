package com.riot.matesense.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.enums.Status;
import com.riot.matesense.enums.ConfidenceQuality;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.Gate;
import com.riot.matesense.model.GateForDownlink;
import com.riot.matesense.repository.GateRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import javax.tools.Diagnostic;
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
                    e.getWorkerConfidence(), e.getRequestedStatus());
            customGates.add(gate);
        });
        return customGates;
    }

    public String addGate(GateEntity gate) throws GateAlreadyExistingException {
        gateRepository.save(gate);
        return gate.toString();
    }

    public void removeGate(GateEntity gate) throws GateNotFoundException {
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
                gate.getSensorConfidence(), gate.getRequestedStatus());
    }

    public void changeConfidence()
    {
        ConfidenceQuality quality = ConfidenceQuality.HIGH;
    }

    /*@Scheduled(fixedRate = 10000)
    public void periodicSubtractConfidence()
    {
        List<GateEntity> gates = gateRepository.findAll();
        gates.forEach(e -> {
            e.getCalculator().subtractConfidence();
            updateGate(e);
        });
    }*/

    public void requestGateStatusChange(Long gateId, String targetStatus) throws GateNotFoundException {
        GateEntity gate = gateRepository.getById(gateId);

        if ((gate.getStatus().toString().strip()).equals(targetStatus)) {
            System.out.println("nothing");
            return;
        }
        if (targetStatus.equals("NONE")) {
            targetStatus = null;
        }
        gate.setRequestedStatus(targetStatus);

        System.out.println(gate.getRequestedStatus());
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

}
