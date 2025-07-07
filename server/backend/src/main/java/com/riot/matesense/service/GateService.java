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

    public void updateGate(GateEntity gate, int reportType) {
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

        changeConfidence(gate, gate.getConfidence(), reportType);
        gate.setConfidence(gate.getConfidence());
        gateRepository.save(gate);

    }

    public boolean existsGateById(Long id) {
        return gateRepository.existsById(Math.toIntExact(id));
    }

    public GateEntity getGateEntityById(Long id) throws GateNotFoundException {
        return gateRepository.findById(Math.toIntExact(id)).orElseThrow(() -> new GateNotFoundException(id));
    }

    public Gate getGateById(Long id) throws GateNotFoundException {
        GateEntity gate = gateRepository.findById(Math.toIntExact(id)).orElseThrow(() -> new GateNotFoundException(id));
        return new Gate(gate.getId(), gate.getDeviceId(), gate.getLastTimeStamp(), gate.getStatus(),
                gate.getLatitude(), gate.getLongitude(), gate.getLocation(), gate.getWorkerConfidence(),
                gate.getSensorConfidence(), gate.getRequestedStatus(), gate.getConfidence(), gate.getQuality());
    }


//    public Gate getGateById(Long id) {
//        GateEntity gate = gateRepository.getById(id);
//        return new Gate(gate.getId(), gate.getDeviceId(), gate.getLastTimeStamp(), gate.getStatus(),
//                gate.getLatitude(), gate.getLongitude(), gate.getLocation(), gate.getWorkerConfidence(),
//                gate.getSensorConfidence(), gate.getRequestedStatus(), gate.getConfidence(), gate.getQuality());
//    }

    public void changeConfidence(GateEntity entity, int passedConfidence, int reportType)
    {
        System.out.println("Passed confidence:" + passedConfidence);
        int confidence;
        Status gateStatus = entity.getStatus();
        //Safety for out of bounds case
        Status[] gateArray = entity.getGateStatusArray() != null ? entity.getGateStatusArray() : new Status[0];
        Status[] workerArray = entity.getWorkerStatusArray() != null ? entity.getWorkerStatusArray() : new Status[0];


        if (gateStatus == Status.UNKNOWN || gateStatus == Status.NONE) //set confidence to max if status doesn't exist or is reported as unknown
        {
            confidence = 100; //(if we don't know, we're sure that we don't know)
        }
        else
        {
            confidence = passedConfidence;
            int iterations = Math.min(5, Math.min(gateArray.length, workerArray.length));

            for (int i = 0; i < iterations; i++)
            {
                if(reportType == 1) // IST STATE
                {
                    int delta = 10 - (2 * i);
                    if (gateStatus == gateArray[i] && gateArray[i] != Status.NONE)
                    {
                        confidence += delta; // if new a report matches an older report, increase confidence
                    }
                    else if (gateArray[i] != Status.NONE)
                    {
                        confidence -= delta; // otherwise, decrease confidence
                    }
                }
                else if(reportType == 2) // SEEN STATE
                {
                    int delta = 20 - (4 * i);
                    if (gateStatus == workerArray[i] && workerArray[i] != Status.NONE)
                    {
                        confidence += delta;
                    }
                    else if (workerArray[i] != Status.NONE)
                    {
                        confidence -= delta;
                    }
                }
                else // update not originating from a gate or worker
                {
                    break;
                }
            }
        }

        entity.shuffleReports(gateStatus, reportType);

        confidence = Math.max(0, confidence); // normalize confidence, between 0 and 100
        confidence = Math.min(100, confidence);

        entity.setConfidence(confidence);

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

        System.out.println("Calculated confidence:" + confidence + "\n");
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
