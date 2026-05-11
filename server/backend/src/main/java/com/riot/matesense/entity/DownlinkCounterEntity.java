package com.riot.matesense.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

/**
 * An Entity to store a counter for the downlinks
 */
@Setter
@Getter
@Entity
@Table(name = "downlink_counter")
public class DownlinkCounterEntity {

    @Id
    private Long id = 1L;

    private int counter = 0;

    public DownlinkCounterEntity() {

    }

}
