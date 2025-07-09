package com.riot.matesense.model;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.Getter;
import lombok.Setter;

@Setter
@Getter
public class DownlinkCounter {
    @JsonProperty
    private Long id = 1L;
    @JsonProperty
    private int counter = 0;

    public DownlinkCounter() {

    }

 }