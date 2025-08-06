package com.riot.matesense.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

@Setter
@Getter
@Entity
@Table(name = "counter_downlink")
public class DownlinkCounterEntity {

    // Getters and setters
    @Id

    private Long id = 1L;

    private int counter = 0;

    public DownlinkCounterEntity() {

    }

}
