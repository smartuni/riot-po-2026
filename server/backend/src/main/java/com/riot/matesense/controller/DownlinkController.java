package com.riot.matesense.controller;

import com.riot.matesense.config.DownPayload;
import com.riot.matesense.service.DownlinkService;
//import jakarta.validation.Valid;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/downlink")
public class DownlinkController {

    private final DownlinkService downlinkService;

    public DownlinkController(DownlinkService downlinkService) {
        this.downlinkService = downlinkService;
    }

    @PostMapping
    public ResponseEntity<String> sendDownlink(
            @RequestBody DownPayload payload){
        downlinkService.sendDownlinkToDevice(payload);
        return ResponseEntity.ok("Downlink vorbereitet.");
    }
}
