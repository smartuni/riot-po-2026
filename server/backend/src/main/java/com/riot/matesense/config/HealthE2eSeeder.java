package com.riot.matesense.config;

import com.riot.matesense.enums.BatteryStatus;
import com.riot.matesense.enums.ShockStatus;
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
            healthStatusService.updateHealth(1001, BatteryStatus.CHARGING, ShockStatus.NO_SHOCK, 4200, 1);
            healthStatusService.updateHealth(1002, BatteryStatus.DISCHARGING, ShockStatus.NO_SHOCK, 3700, 1);
            healthStatusService.updateHealth(1003, BatteryStatus.LOW_BATTERY, ShockStatus.NO_SHOCK, 3200, 1);
            healthStatusService.updateHealth(1004, BatteryStatus.DISCHARGING, ShockStatus.SHOCK_DETECTED, 3500, 1);
        };
    }
}
