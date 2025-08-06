package com.riot.matesense.config;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.Getter;
import lombok.Setter;
import com.fasterxml.jackson.annotation.JsonFormat;

import java.sql.Timestamp;
import java.util.List;

@Getter @Setter
@JsonFormat(shape = JsonFormat.Shape.ARRAY)
public class DownPayload {
    private int messageType;
    private Timestamp timestamp;
    private List<List<Integer>> statuses;

    @JsonCreator
    public DownPayload(
            @JsonProperty(index = 0) int messageType,
            @JsonProperty(index = 1) Timestamp timestamp,
            @JsonProperty(index = 2) List<List<Integer>> statuses
    ) {
        this.messageType = messageType;
        this.timestamp = timestamp;
        this.statuses = statuses;
    }
}


