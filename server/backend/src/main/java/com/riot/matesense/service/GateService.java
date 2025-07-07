package com.riot.matesense.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.riot.matesense.entity.GateActivityEntity;
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

    
    public GateService(GateRepository gateRepository, SimpMessagingTemplate messagingTemplate) {
        this.gateRepository = gateRepository;
        this.messagingTemplate = messagingTemplate;
    }

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

    public String addGate(GateEntity gate) throws GateAlreadyExistingException {
        gateRepository.save(gate);
        messagingTemplate.convertAndSend("/topic/gate-activities/add", gate);
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
                gate.getSensorConfidence(), gate.getRequestedStatus(), gate.getConfidence(), gate.getQuality(), gate.getPendingJob(), gate.getPriority());
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


//    public Gate getGateById(Long id) {
//        GateEntity gate = gateRepository.getById(id);
//        return new Gate(gate.getId(), gate.getDeviceId(), gate.getLastTimeStamp(), gate.getStatus(),
//                gate.getLatitude(), gate.getLongitude(), gate.getLocation(), gate.getWorkerConfidence(),
//                gate.getSensorConfidence(), gate.getRequestedStatus(), gate.getConfidence(), gate.getQuality());
//    }

    /*@Scheduled(fixedRate = 10000)
    public void periodicSubtractConfidence()
    {
        List<GateEntity> gates = gateRepository.findAll();
        gates.forEach(e -> {
            e.getCalculator().subtractConfidence();
            updateGate(e);
        });
    }*/


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

    public void changeGateStatus(Long gateId, int status) {
        GateEntity gate = gateRepository.getById(gateId);
        System.out.println("Current Status: " + gate.getStatus());
        // System.out.println("Requested Status: " + targetStatus);
        System.out.println("ID: " + gate.getId());

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
        gate.setStatus(Status.fromCode(status));
        changeConfidence(gate, status, 1);
        messagingTemplate.convertAndSend("/topic/gates/updates", gate);
            // }
        // }

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

    public String addGateFromGUI(GateEntity gate) throws GateAlreadyExistingException {
        gate.setPriority(3);
        gate.setLastTimeStamp(new Timestamp(System.currentTimeMillis()));
        //TODO: if confidence calc is done remove this
        gate.setConfidence(100);
        gateRepository.save(gate);
        //TODO: IS THIS THE RIGHT WAY?
        messagingTemplate.convertAndSend("/topic/gates/add", gate);
        // Notify all clients about the new gate
        return gate.toString();
    }
}
