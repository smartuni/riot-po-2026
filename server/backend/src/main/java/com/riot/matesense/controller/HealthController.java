package com.riot.matesense.controller;

import com.riot.matesense.model.HealthStatusDTO;
import com.riot.matesense.service.HealthStatusService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.Map;

@RestController
@RequestMapping("/health")
public class HealthController {

    private final HealthStatusService healthStatusService;

    public HealthController(HealthStatusService healthStatusService) {
        this.healthStatusService = healthStatusService;
    }

    @GetMapping
    public ResponseEntity<Map<Integer, HealthStatusDTO>> getAllHealth() {
        return ResponseEntity.ok(healthStatusService.getAll());
    }
}
