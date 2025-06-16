package com.riot.matesense.config;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.core.JsonParser;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.*;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.riot.matesense.model.GateForDownlink;
import lombok.Getter;
import lombok.Setter;
import com.fasterxml.jackson.annotation.JsonFormat;
import java.util.List;

@Getter @Setter
@JsonFormat(shape = JsonFormat.Shape.ARRAY)
public class DownPayload {
    private int messageType;
    private int timestamp;
    private List<List<Integer>> statuses;

    @JsonCreator
    public DownPayload(
            @JsonProperty(index = 0) int messageType,
            @JsonProperty(index = 1) int timestamp,
            @JsonProperty(index = 2) List<List<Integer>> statuses
    ) {
        this.messageType = messageType;
        this.timestamp = timestamp;
        this.statuses = statuses;
    }
}


