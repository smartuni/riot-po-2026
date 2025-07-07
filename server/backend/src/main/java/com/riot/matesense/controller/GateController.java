package com.riot.matesense.controller;

import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.Gate;
import com.riot.matesense.model.GateForDownlink;
import com.riot.matesense.service.GateService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;


@RestController
// @CrossOrigin(origins = "*")
public class GateController {

    @Autowired
    GateService gateService;

    @RequestMapping(value = "gates", method = RequestMethod.GET)
    public List<Gate> getAllGates() {
        return gateService.getAllGates();
    }

    @RequestMapping(value = "add-gate", method = RequestMethod.POST)
    public String addGate(@RequestBody GateEntity gateEntity) throws GateAlreadyExistingException {
        return gateService.addGate(gateEntity);
    }

    @RequestMapping(value = "update-gate", method = RequestMethod.PUT)
    public void updateGate(@RequestBody GateEntity gate) throws GateNotFoundException {
        gateService.updateGate(gate, 3);
    }

    @RequestMapping(value = "delete-gate", method = RequestMethod.DELETE)
    public void removeCustomer(@RequestBody GateEntity gate) throws GateNotFoundException {
        gateService.removeGate(gate);
    }

    @PostMapping("/{gateId}/request-status-change")
    public void requestGateStatusChange(@PathVariable Long gateId, @RequestBody Map<String, String> body)
            throws GateNotFoundException {
        String targetStatus = body.get("requestedStatus");
        gateService.requestGateStatusChange(gateId, targetStatus);
    }

    @RequestMapping(value = "gates_for_downlink", method = RequestMethod.GET)
    public List<GateForDownlink> getAllGatesForDownlink() {
        return gateService.getAllGatesForDownlink();
    }


}
