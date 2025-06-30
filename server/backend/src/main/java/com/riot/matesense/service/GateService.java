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
            System.out.println("getAll");
            // changeConfidence(e, (int)(Math.random() * 100));
            Gate gate = new Gate(e.getId(), e.getDeviceId(), e.getLastTimeStamp(), e.getStatus(),
                    e.getLatitude(), e.getLongitude(), e.getLocation(), e.getSensorConfidence(),
                    e.getWorkerConfidence(), e.getRequestedStatus(), e.getConfidence(), e.getQuality());
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
            gateEntity.setGateStatusArray(gate.getGateStatusArray());
            gateEntity.setWorkerStatusArray(gate.getWorkerStatusArray());
            System.out.println("update");
            changeConfidence(gateEntity, gate.getConfidence());
            gateRepository.save(gateEntity);
        }

    }

    public Gate getGateById(Long id) {
        GateEntity gate = gateRepository.getById(id);
        return new Gate(gate.getId(), gate.getDeviceId(), gate.getLastTimeStamp(), gate.getStatus(),
                gate.getLatitude(), gate.getLongitude(), gate.getLocation(), gate.getWorkerConfidence(),
                gate.getSensorConfidence(), gate.getRequestedStatus(), gate.getConfidence(), gate.getQuality());
    }

    public void changeConfidence(GateEntity entity, int passedConfidence)
    {
        System.out.println("Passed confidence:" + passedConfidence);
        int confidence;
        Status gateStatus = entity.getStatus();
        Status[] gateArray = entity.getGateStatusArray();
        Status[] workerArray = entity.getWorkerStatusArray();

        if (gateStatus == Status.UNKNOWN || gateStatus == Status.NONE) //set confidence to max if status doesn't exist or is reported as unknown
        {
            confidence = 100; //(if we don't know, we're sure that we don't know)
        }
        else
        {
            confidence = passedConfidence;
            for(int i = 0; i < 5; i++)
            {
                int gateDelta = 10 - (2 * i);
                int workerDelta = 20 - (4 * i);

                if (gateStatus == gateArray[i] && gateArray[i] != Status.NONE)
                {
                    confidence += gateDelta; // if new a report matches an older report, increase confidence
                }
                else if (gateArray[i] != Status.NONE)
                {
                    confidence -= gateDelta; // otherwise, decrease confidence
                }

                if (gateStatus == workerArray[i] && workerArray[i] != Status.NONE)
                {
                    confidence += workerDelta;
                }
                else if (workerArray[i] != Status.NONE)
                {
                    confidence -= workerDelta;
                }
            }
        }
        
        entity.setConfidence(confidence);
        entity.shuffleReports(gateStatus);

        confidence = Math.max(0, confidence); // normalize confidence, between 0 and 100
        confidence = Math.min(100, confidence);

        if (confidence >= 90){
            entity.setQuality(ConfidenceQuality.HIGH);
        }
        else if (confidence >= 80 && confidence < 90){
            entity.setQuality(ConfidenceQuality.MED_HIGH);
        }
        else if (confidence >= 70 && confidence < 80){
            entity.setQuality(ConfidenceQuality.MED);
        }
        else if (confidence >= 60 && confidence < 70){
            entity.setQuality(ConfidenceQuality.MED_LOW);
        }
        else {
            entity.setQuality(ConfidenceQuality.LOW);
        }

        System.out.println("Calculated confidence:" + passedConfidence + "\n");
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
