package com.riot.matesense.model;

import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;
import java.util.List;
@Getter
@Setter
public class DownPayload {
    private int id;
    private long timestamp = new Timestamp(System.currentTimeMillis()).getTime();
    private List<List<GateForDownlink>> gates; // List of gates with their IDs and statuses
}
