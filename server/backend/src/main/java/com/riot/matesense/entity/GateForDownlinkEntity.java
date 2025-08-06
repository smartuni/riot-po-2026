package com.riot.matesense.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

/**
 * A helper Entity for storing only the necessary values for the MQTT-Stuff
 */
@Getter
@Setter
@Table(name = "gates_for_downlink")
@Entity
public class GateForDownlinkEntity {
    @Id
    private int id;
    @Column(name = "requested_status")
    private int requestedStatus;

    public GateForDownlinkEntity(int id, int requestedStatus) {
        this.id = id;
        this.requestedStatus = requestedStatus;
    }

    public GateForDownlinkEntity() {
    }
}