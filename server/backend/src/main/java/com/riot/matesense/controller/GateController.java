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
import java.util.ArrayList;
import java.util.List;
import java.util.Map;


@RestController
// @CrossOrigin(origins = "*")
public class GateController {

    @Autowired
    GateService gateService;
    @Autowired
    GateActivityService gateActivityService;

    @RequestMapping(value = "gates", method = RequestMethod.GET)
    public List<Gate> getAllGates() {
        return gateService.getAllGates();
    }

    @RequestMapping(value = "add-gate", method = RequestMethod.POST)
    public String addGate(@RequestBody GateEntity gateEntity) throws GateAlreadyExistingException {
        return gateService.addGate(gateEntity);
    }

    @RequestMapping(value = "add-gate-ui", method = RequestMethod.POST)
    public String addGateUI(@RequestBody GateEntity gateEntity) throws GateAlreadyExistingException {
        return gateService.addGateFromGUI(gateEntity);
    }

    @RequestMapping(value = "update-gate", method = RequestMethod.PUT)
    public void updateGate(@RequestBody GateEntity gate) throws GateNotFoundException {
        gateService.updateGate(gate, MsgType.DUMMY_STATE);
    }

    @DeleteMapping("/gates/{id}")
    public void deleteGate(@PathVariable Long id) throws GateNotFoundException {
        gateService.removeGateById(id);
    }


    @PostMapping("/{gateId}/{workerId}/request-status-change/")
    public void requestGateStatusChange(@PathVariable Long gateId, @PathVariable Long workerId,  @RequestBody Map<String, String> body)
            throws GateNotFoundException {
        String targetStatus = body.get("requestedStatus");
        gateService.requestGateStatusChange(gateId, targetStatus);
        gateActivityService.addGateActivity(new GateActivityEntity(new Timestamp(System.currentTimeMillis()), gateId, targetStatus, "The worker with ID: " + workerId + " requested the Status: "+ targetStatus + " to the gate with Gate-ID: " +gateId, workerId));
    }

    @RequestMapping(value = "gates_for_downlink", method = RequestMethod.GET)
    public List<GateForDownlink> getAllGatesForDownlink() {
        return gateService.getAllGatesForDownlink();
    }

    @PutMapping("/update-priority/{gateId}")
    public void updatePriority(
            @PathVariable Long gateId,
            @RequestBody Map<String, Integer> request
    ) {
        Integer priority = request.get("priority");
        gateService.updatePriority(gateId, priority);
    }

}
