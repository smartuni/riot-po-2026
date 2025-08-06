package com.riot.matesense.controller;

import com.riot.matesense.entity.DownlinkCounterEntity;
import com.riot.matesense.entity.NotificationEntity;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.GateActivity;
import com.riot.matesense.model.Notification;
import com.riot.matesense.service.DownlinkCounterService;
import com.riot.matesense.service.NotificationService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/downlinkcounter")
    public class DownlinkCounterController {
        @Autowired
        DownlinkCounterService counterService;

    @RequestMapping(value = "/counter", method = RequestMethod.GET)
    public int getCounter(){
        return counterService.getCounter();
    }

    @PostMapping("/try-increment")
    public boolean tryIncrement() {
        return counterService.incrementCounterIfBelowLimit();
    }

    @PostMapping("/reset")
    public void resetCounter() {
        counterService.resetCounter();
    }
}
