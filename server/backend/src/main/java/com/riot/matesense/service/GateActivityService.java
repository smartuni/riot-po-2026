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
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

@Service
public class GateActivityService {

    @Autowired
    GateActivityRepository gateActivityRepository;

    public List<GateActivity> getAllGateActivities() {
        List<GateActivityEntity> gates = gateActivityRepository.findAll();
        List<GateActivity> customGateActivities = new ArrayList<>();
        gates.forEach(e -> {
            GateActivity gateActivity = new GateActivity(e.getLastTimeStamp(), e.getGateId(), e.getRequestedStatus(), e.getMessage(), e.getId(), e.getWorkerId());
            customGateActivities.add(gateActivity);
        });
        return customGateActivities;
    }

    public String addGateActivity(GateActivityEntity gateActivity) {
        gateActivityRepository.save(gateActivity);
        return gateActivity.toString();
    }

    public void addGateActivities(List<GateActivityEntity> gates) {
        for (GateActivityEntity gate : gates) {
            addGateActivity(gate);
        }
    }


    public void removeGateActivity(GateActivityEntity gateActivityEntity) {
        gateActivityRepository.delete(gateActivityEntity);
    }


    public List<GateActivity> getGateActivitiesByGateId(Long gateId) {
        List<GateActivityEntity> gateActivities = gateActivityRepository.findAll().stream().filter(e -> e.getGateId().equals(gateId)).collect(Collectors.toList());
        List<GateActivity> customGateActivities = new ArrayList<>();
        for (GateActivityEntity gateActivityEntity : gateActivities) {
            GateActivity gateActivity = new GateActivity(gateActivityEntity.getLastTimeStamp(), gateActivityEntity.getGateId(), gateActivityEntity.getRequestedStatus(), gateActivityEntity.getMessage(), gateActivityEntity.getId(), gateActivityEntity.getWorkerId());
            customGateActivities.add(gateActivity);
        }
        return customGateActivities;
    }

    public String buildStatusMessage(GateActivity gateActivity) {
        return String.format("Gate ID: %d, Last Timestamp: %s, Requested Status: %s",
                gateActivity.getGateId(),
                gateActivity.getLastTimeStamp().toString(),
                gateActivity.getRequestedStatus());
    }
}

