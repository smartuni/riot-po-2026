package com.riot.matesense.config;

import com.riot.matesense.enums.BatteryStatus;
import com.riot.matesense.enums.FreeFallStatus;
import com.riot.matesense.service.HealthStatusService;
import org.springframework.boot.ApplicationRunner;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Profile;

/**
 * Seeds initial health data for the e2e profile so the frontend
 * has health data to display immediately on page load.
 */
@Configuration
@Profile("e2e")
public class HealthE2eSeeder {

    @Bean
    public ApplicationRunner seedHealthData(HealthStatusService healthStatusService) {
        return args -> {
            healthStatusService.updateHealth(1001, BatteryStatus.CHARGING, FreeFallStatus.NO_FALL, 4200, 1);
            healthStatusService.updateHealth(1002, BatteryStatus.DISCHARGING, FreeFallStatus.NO_FALL, 3700, 1);
            healthStatusService.updateHealth(1003, BatteryStatus.LOW_BATTERY, FreeFallStatus.NO_FALL, 3200, 1);
            healthStatusService.updateHealth(1004, BatteryStatus.DISCHARGING, FreeFallStatus.FREE_FALL_DETECTED, 3500, 1);
        };
    }
}
