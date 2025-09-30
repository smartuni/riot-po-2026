package com.riot.matesense.service;

import com.riot.matesense.enums.StateConfirmation;
import com.riot.matesense.enums.Status;
import com.riot.matesense.entity.GateActivityEntity;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.GateActivity;
import com.riot.matesense.repository.GateActivityRepository;

import com.riot.matesense.repository.WorkerIdView;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

@Service
public class GateActivityService {

    @Autowired
    GateActivityRepository gateActivityRepository;

    @Autowired
    GateService gateService;
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
            GateActivity gateActivity = new GateActivity(e.getLocalTimeStamp(), e.getGateTimeStamp(), e.getGateId(), e.getRequestedStatus(), e.getMessage(), e.getId(), e.getWorkerId());
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

        if (saved.getWorkerId() != null) {
            try {
                GateEntity gate = gateService.getGateEntityById(saved.getGateId());
                Status gateState = gate.getStatus();
                List<GateActivity> gas = this.getLatestGateActivitiesByGateId(gateActivity.getGateId());
                //TODO filter list for *relevant* reports (report time after last gate state change)
                //gas.removeIf(ga -> ga.)
                int conflicts = 0;
                for (GateActivity g : gas) {
                    if (!g.getRequestedStatus().equals(gateState.toString())) {
                        conflicts++;
                        gateService.changeGateStateConfirmation(gate.getId(), StateConfirmation.WORKER_CONFLICT);
                    }
                }
                if (conflicts == 0) {
                    if (gas.size() == 0) {
                        gateService.changeGateStateConfirmation(gate.getId(), StateConfirmation.UNCONFIRMED);
                    } else if (gas.size() == 1) {
                        gateService.changeGateStateConfirmation(gate.getId(), StateConfirmation.WORKER_CONFIRMED_SINGLE);
                    } else if (gas.size() > 1) {
                        gateService.changeGateStateConfirmation(gate.getId(), StateConfirmation.WORKER_CONFIRMED_MULTI);
                    } // TODO: add check for "all active SenseMates confirmed"
                }
            } catch (GateNotFoundException e) {
                System.err.println(e.getMessage());
                return gateActivity.toString();
            }
        }
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
        List<GateActivityEntity> gateActivities = gateActivityRepository.findAll().stream().filter(e -> e.getGateId().equals(gateId)).toList();
        List<GateActivity> customGateActivities = new ArrayList<>();
        for (GateActivityEntity gateActivityEntity : gateActivities) {
            GateActivity gateActivity = new GateActivity(gateActivityEntity.getLocalTimeStamp(), gateActivityEntity.getGateTimeStamp(), gateActivityEntity.getGateId(), gateActivityEntity.getRequestedStatus(), gateActivityEntity.getMessage(), gateActivityEntity.getId(), gateActivityEntity.getWorkerId());
            customGateActivities.add(gateActivity);
        }
        return customGateActivities;
    }

    /**
     * For the given gate id returns the latest activities which resemble seen-reports.
     * This only includes reports which originate from a SenseMate. For each SenseMate only the latest report about the given gate is returned.
     * @param gateId from the gate
     * @return the latest activities
     */
    public List<GateActivity> getLatestGateActivitiesByGateId(Long gateId) {
        //List<GateActivityEntity> gateActivities = gateActivityRepository.findAll().stream().filter(e -> e.getGateId().equals(gateId) && (e.getWorkerId() != null) ).sorted(Comparator.reverseOrder()).collect(Collectors.toList());
        //List<GateActivity> customGateActivities = new ArrayList<>();
        //for (GateActivityEntity gateActivityEntity : gateActivities) {
        //    GateActivity gateActivity = new GateActivity(gateActivityEntity.getLocalTimeStamp(), gateActivityEntity.getGateTimeStamp(), gateActivityEntity.getGateId(), gateActivityEntity.getRequestedStatus(), gateActivityEntity.getMessage(), gateActivityEntity.getId(), gateActivityEntity.getWorkerId());
        //    customGateActivities.add(gateActivity);
        //}
        //return customGateActivities;
        List<Long> senseMateIDs = new ArrayList<Long>();
        List<WorkerIdView> workers = gateActivityRepository.findDistinctWorkerIdByGateId(gateId).stream().filter(Objects::nonNull).toList();
        System.out.println("########################");
        System.out.println(workers);
        List<GateActivity> customGateActivities = new ArrayList<>();
        for (WorkerIdView w: workers) {
            System.out.println(w.getWorkerId());
            List<GateActivityEntity> gateActivities = gateActivityRepository.findAll().stream().filter(
                    e -> e.getGateId().equals(gateId) && (e.getWorkerId() != null) && (e.getWorkerId().equals(w.getWorkerId())))
                    .sorted(Comparator.reverseOrder()).limit(1).toList();
            System.out.println(gateActivities);

            for (GateActivityEntity gae: gateActivities) {
                customGateActivities.add(new GateActivity(gae.getLocalTimeStamp(),
                        gae.getGateTimeStamp(),
                        gae.getGateId(),
                        gae.getRequestedStatus(),
                        gae.getMessage(),
                        gae.getId(),
                        gae.getWorkerId()));
            }

        }
        return customGateActivities;
    }

    /**
     * A method to build a specific message for the gateActivity
     * @param gateActivity
     * @return a messageString
     */
    public String buildStatusMessage(GateActivity gateActivity) {
        return String.format("Gate ID: %d, Local Timestamp: %s, Requested Status: %s",
                gateActivity.getGateId(),
                gateActivity.getLocalTimeStamp().toString(),
                gateActivity.getRequestedStatus());
    }
}

