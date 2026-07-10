package com.riot.matesense.controller;

import com.riot.matesense.enums.BatteryStatus;
import com.riot.matesense.enums.FreeFallStatus;
import com.riot.matesense.model.HealthStatusDTO;
import com.riot.matesense.service.HealthStatusService;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.setup.MockMvcBuilders;

import java.util.Map;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.times;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

class HealthControllerTest {

    private MockMvc mockMvc;
    private HealthStatusService healthStatusService;

    @BeforeEach
    void setUp() {
        healthStatusService = mock(HealthStatusService.class);
        mockMvc = MockMvcBuilders.standaloneSetup(new HealthController(healthStatusService)).build();
    }

    @Test
    void getAllHealth_returnsOkAndJsonBody_whenServiceHasData() throws Exception {
        // Given: service holds one health entry for gate 1
        HealthStatusDTO dto = new HealthStatusDTO(
                1, 1, FreeFallStatus.NO_FALL, BatteryStatus.CHARGING, 4200);
        when(healthStatusService.getAll()).thenReturn(Map.of(1, dto));

        // When + Then: GET /health returns 200 with correct JSON
        mockMvc.perform(get("/health"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.1.senseGateId").value(1))
                .andExpect(jsonPath("$.1.batteryStatus").value("CHARGING"))
                .andExpect(jsonPath("$.1.freeFallStatus").value("NO_FALL"))
                .andExpect(jsonPath("$.1.voltageMv").value(4200))
                .andExpect(jsonPath("$.1.version").value(1));
    }

    @Test
    void getAllHealth_returnsOkAndEmptyMap_whenServiceHasNoData() throws Exception {
        // Given: service store is empty
        when(healthStatusService.getAll()).thenReturn(Map.of());

        // When + Then: GET /health returns 200 with empty JSON object
        mockMvc.perform(get("/health"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$").isEmpty());
    }

    @Test
    void getAllHealth_returnsMultipleEntries_whenServiceHasMultipleGates() throws Exception {
        // Given: service holds health entries for two gates
        HealthStatusDTO dto1 = new HealthStatusDTO(
                1, 1, FreeFallStatus.NO_FALL, BatteryStatus.CHARGING, 4200);
        HealthStatusDTO dto2 = new HealthStatusDTO(
                1, 2, FreeFallStatus.FREE_FALL_DETECTED, BatteryStatus.LOW_BATTERY, 3200);
        when(healthStatusService.getAll()).thenReturn(Map.of(1, dto1, 2, dto2));

        // When + Then: GET /health returns both entries keyed by senseGateId
        mockMvc.perform(get("/health"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.1.senseGateId").value(1))
                .andExpect(jsonPath("$.2.senseGateId").value(2))
                .andExpect(jsonPath("$.2.freeFallStatus").value("FREE_FALL_DETECTED"))
                .andExpect(jsonPath("$.2.batteryStatus").value("LOW_BATTERY"));
    }

    @Test
    void getAllHealth_callsServiceExactlyOnce_perRequest() throws Exception {
        // Given
        when(healthStatusService.getAll()).thenReturn(Map.of());

        // When
        mockMvc.perform(get("/health"));

        // Then: service is consulted exactly once (not cached or skipped)
        verify(healthStatusService, times(1)).getAll();
    }
}
