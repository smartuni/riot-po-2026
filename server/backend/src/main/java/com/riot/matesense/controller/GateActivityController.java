
package com.riot.matesense.controller;

import com.riot.matesense.entity.GateActivityEntity;
import com.riot.matesense.model.GateActivity;
import com.riot.matesense.service.GateActivityService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
// @CrossOrigin(origins = "*")
public class GateActivityController {

	@Autowired
	GateActivityService gateActivityService;
	
	@RequestMapping(value = "gate-activities", method = RequestMethod.GET)
	public List<GateActivity> getAllGates(){
		return gateActivityService.getAllGateActivities();
	}
	
	@RequestMapping(value = "add-activity", method = RequestMethod.POST)
	public String addGate(@RequestBody GateActivityEntity gateActivityEntity){
		return gateActivityService.addGateActivity(gateActivityEntity);
	}

	@RequestMapping(value = "add-activities", method = RequestMethod.POST)
	public void addGate(@RequestBody List<GateActivityEntity> gateActivityEntity){
		gateActivityService.addGateActivities(gateActivityEntity);
	}
	
	@RequestMapping(value = "delete-activitiy", method = RequestMethod.DELETE)
	public void removeCustomer(@RequestBody GateActivityEntity gateActivityEntity){
		gateActivityService.removeGateActivity(gateActivityEntity);
	}

	@RequestMapping(value = "activities/{gateId}", method = RequestMethod.GET)
	public List<GateActivity> getGateActivitiesByGateId(@PathVariable Long gateId) {
		return gateActivityService.getGateActivitiesByGateId(gateId);
	}

}

