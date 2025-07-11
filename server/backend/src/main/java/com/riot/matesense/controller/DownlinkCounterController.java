package com.riot.matesense.controller;

import com.riot.matesense.service.DownlinkCounterService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

/**
 * A Controller for the Downlink-Counter
 * A Downlink-Counter is a counter that is stored in the database which tracks the sent downlinks
 */
@RestController
@RequestMapping("/downlinkcounter")
public class DownlinkCounterController {
    @Autowired
    DownlinkCounterService counterService;

    /**
     * A API call to get the counter off the Downlink-Counter
     * @return the counter of the class
     */
    @RequestMapping(value = "/counter", method = RequestMethod.GET)
    public int getCounter(){
        return counterService.getCounter();
    }

    /**
     * A API call to increment the counter of the Downlink-Counter Entity
     * @return a boolean if the counter is successfully increased
     */

    @PostMapping("/try-increment")
    public boolean tryIncrement() {
        return counterService.incrementCounterIfBelowLimit();
    }

    /**
     * A API Call to reset the counter of the Downlink-Counter Entity
     */
    @PostMapping("/reset")
    public void resetCounter() {
        counterService.resetCounter();
    }
}
