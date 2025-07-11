package com.riot.matesense.service;

import com.riot.matesense.entity.GateActivityEntity;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.Gate;
import com.riot.matesense.model.GateActivity;
import com.riot.matesense.model.GateForDownlink;
import com.riot.matesense.repository.GateActivityRepository;
import com.riot.matesense.repository.GateRepository;
import com.riot.matesense.repository.NotificationRepository;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

@Service
public class GateActivityService {

    @Autowired
    GateActivityRepository gateActivityRepository;
    private SimpMessagingTemplate messagingTemplate;

    public GateActivityService(GateActivityRepository gateActivityRepository,
                               SimpMessagingTemplate messagingTemplate) {
        this.gateActivityRepository = gateActivityRepository;
        this.messagingTemplate = messagingTemplate;
    }

    /**
     * a method to get all the gateActivityEntities
     * @return a list with all the gateActivities
     */
    public List<GateActivity> getAllGateActivities() {
        List<GateActivityEntity> gates = gateActivityRepository.findAll();
        List<GateActivity> customGateActivities = new ArrayList<>();
        gates.forEach(e -> {
            GateActivity gateActivity = new GateActivity(e.getLastTimeStamp(), e.getGateId(), e.getRequestedStatus(), e.getMessage(), e.getId(), e.getWorkerId());
            customGateActivities.add(gateActivity);
        });
        return customGateActivities;
    }

    /**
     * a method to add a gateActivity to the repository(database)
     * @param gateActivity the activity that needs to be added
     * @return the gate as a String
     */
    public String addGateActivity(GateActivityEntity gateActivity) {
        GateActivityEntity saved = gateActivityRepository.save(gateActivity);
        messagingTemplate.convertAndSend("/topic/gate-activities", saved);
        return gateActivity.toString();
    }

    /**
     * a method to add a List of gateActivities
     * @param gates to be added
     */
    public void addGateActivities(List<GateActivityEntity> gates) {
        for (GateActivityEntity gate : gates) {
            addGateActivity(gate);
        }
    }

    /**
     * a method to remove a gateActivity from the database
     * @param gateActivityEntity to remove from the database
     */
    public void removeGateActivity(GateActivityEntity gateActivityEntity) {
        gateActivityRepository.delete(gateActivityEntity);
        messagingTemplate.convertAndSend("/topic/gate-activities/delete", gateActivityEntity.getId());
    }

    /**
     * a method to get a gateActivity by the GateID
     * @param gateId from the gate
     * @return the activity
     */
    public List<GateActivity> getGateActivitiesByGateId(Long gateId) {
        List<GateActivityEntity> gateActivities = gateActivityRepository.findAll().stream().filter(e -> e.getGateId().equals(gateId)).collect(Collectors.toList());
        List<GateActivity> customGateActivities = new ArrayList<>();
        for (GateActivityEntity gateActivityEntity : gateActivities) {
            GateActivity gateActivity = new GateActivity(gateActivityEntity.getLastTimeStamp(), gateActivityEntity.getGateId(), gateActivityEntity.getRequestedStatus(), gateActivityEntity.getMessage(), gateActivityEntity.getId(), gateActivityEntity.getWorkerId());
            customGateActivities.add(gateActivity);
        }
        return customGateActivities;
    }

    /**
     * A method to build a specific message for the gateActivity
     * @param gateActivity
     * @return a messageString
     */
    public String buildStatusMessage(GateActivity gateActivity) {
        return String.format("Gate ID: %d, Last Timestamp: %s, Requested Status: %s",
                gateActivity.getGateId(),
                gateActivity.getLastTimeStamp().toString(),
                gateActivity.getRequestedStatus());
    }
}

