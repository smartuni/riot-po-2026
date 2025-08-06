package com.riot.matesense.controller;

import com.riot.matesense.entity.GateActivityEntity;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.Gate;
import com.riot.matesense.model.GateForDownlink;
import com.riot.matesense.service.GateActivityService;
import com.riot.matesense.service.GateService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.sql.Timestamp;
import java.util.List;
import java.util.Map;

/**
 * A Controller for the Downlink-Counter
 * A Downlink-Counter is a counter that is stored in the database which tracks the sent downlinks
 */

@RestController
// @CrossOrigin(origins = "*")
public class GateController {

    @Autowired
    GateService gateService;
    @Autowired
    GateActivityService gateActivityService;


    /**
     * AN API Call to get all the gates that are stored in the database
     * @return
     */
    @RequestMapping(value = "gates", method = RequestMethod.GET)
    public List<Gate> getAllGates() {
        return gateService.getAllGates();
    }


    /**
     * AN API Call to add a Gate to the Database
     * @param gateEntity the Gate that should be added
     * @return returns the added gate
     * @throws GateAlreadyExistingException
     */
    @RequestMapping(value = "add-gate-ui", method = RequestMethod.POST)
    public String addGateUI(@RequestBody GateEntity gateEntity) throws GateAlreadyExistingException {
        return gateService.addGateFromGUI(gateEntity);
    }

    /**
     * AN API Call to update a Gate
     * @param gate that should be updatet
     * @throws GateNotFoundException
     */
    @RequestMapping(value = "update-gate", method = RequestMethod.PUT)
    public void updateGate(@RequestBody GateEntity gate) throws GateNotFoundException {
        gateService.updateGate(gate, MsgType.DUMMY_STATE);
    }


    /**
     * AN API Call to delete a Gate through the ID from it
     * @param id of the gate that should be deletet
     * @throws GateNotFoundException
     */
    @DeleteMapping("/gates/{id}")
    public void deleteGate(@PathVariable Long id) throws GateNotFoundException {
        gateService.removeGateById(id);
    }

    /**
     * AN API Call to change the requested status of the Gate
     * @param gateId of the gate that should be changed
     * @param workerId of the worker that wants to change the status
     * @param body the requested statuses of the gates
     * @throws GateNotFoundException
     */
    @PostMapping("/{gateId}/{workerId}/request-status-change/")
    public void requestGateStatusChange(@PathVariable Long gateId, @PathVariable Long workerId,  @RequestBody Map<String, String> body)
            throws GateNotFoundException {
        String targetStatus = body.get("requestedStatus");
        gateService.requestGateStatusChange(gateId, targetStatus);
        gateActivityService.addGateActivity(new GateActivityEntity(new Timestamp(System.currentTimeMillis()), gateId, targetStatus, "The worker with ID: " + workerId + " requested the Status: "+ targetStatus + " to the gate with Gate-ID: " +gateId, workerId));
    }

    /**
     * AN API Call to get all the GateForDownLink Entities in a List
     * @return the List of all the GateForDownLink Entities
     */
    @RequestMapping(value = "gates_for_downlink", method = RequestMethod.GET)
    public List<GateForDownlink> getAllGatesForDownlink() {
        return gateService.getAllGatesForDownlink();
    }


    /**
     * AN API Call to update the priority of a Gate
     * @param gateId of the gate
     * @param request the data
     */
    @PutMapping("/update-priority/{gateId}")
    public void updatePriority(
            @PathVariable Long gateId,
            @RequestBody Map<String, Integer> request
    ) {
        Integer priority = request.get("priority");
        gateService.updatePriority(gateId, priority);
    }


}
