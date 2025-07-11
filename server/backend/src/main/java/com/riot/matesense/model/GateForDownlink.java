package com.riot.matesense.model;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

/**
 * A DTO for the GateForDownlink Entity
 */
@Setter
@Getter
public class GateForDownlink {
    @JsonProperty
    private int id;
    @JsonProperty
    private int requestedStatus;

    public GateForDownlink(int id, int requestedStatus) {
        this.id = id;
        this.requestedStatus = requestedStatus;
    }

    public GateForDownlink() {       // Default constructor
        }}