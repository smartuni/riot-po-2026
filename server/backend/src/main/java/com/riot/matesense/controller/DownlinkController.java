package com.riot.matesense.controller;

import com.riot.matesense.service.DownlinkService;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/downlink")
public class DownlinkController {

    private final DownlinkService downlinkService;

    public DownlinkController(DownlinkService downlinkService) {
        this.downlinkService = downlinkService;
    }

    @PostMapping("/send")
    public String sende() {
        downlinkService.sendeDownlink();
        return "Downlink gesendet";
    }
}

