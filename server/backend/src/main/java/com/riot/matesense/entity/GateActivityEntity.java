package com.riot.matesense.entity;

import com.riot.matesense.enums.ActivityType;
import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

/**
 * An Entity for the Gate-Activities
 * Everything that is done with a Gate for example: the gate has gotten closed should create a Gate-Activity
 * Another Example: The requested Status of the Gate with ID: 1 has changed to "REQUESTED-OPEN"
 */
@Getter
@Setter
@Table(name = "gate_activities")
@Entity
public class GateActivityEntity implements Comparable<GateActivityEntity> {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    private Timestamp lastTimeStamp;
    private Timestamp localTimeStamp;
    private Timestamp gateTimeStamp;
    private Long gateId;
    private String requestedStatus;
    private String message;
    private Long workerId;
    private ActivityType activityType;

    public GateActivityEntity(Timestamp localTimeStamp, Timestamp gateTimeStamp, Long gateId,
                              String state, ActivityType activityType, Long workerId) {
        this.lastTimeStamp = localTimeStamp;
        this.localTimeStamp = localTimeStamp;
        this.gateTimeStamp = gateTimeStamp;
        this.gateId = gateId;
        this.requestedStatus = state;
        this.workerId = workerId;
        this.activityType = activityType;
        switch (activityType) {
            case SENSOR_NEW -> this.message = "New Gate " + gateId + " has been added with status " + state.toString();
            case SENSOR_VALUE_CHANGED -> this.message = "Gate " + gateId + " has changed to status " + state.toString();
            case SENSOR_VALUE_KEEPALIVE -> this.message = "Gate " + gateId + " reported its state as " + state.toString();
            case SENSEMATE_WORKER_REPORT -> this.message = "Gate "+ gateId + " was reported as " + state.toString() + " by SenseMate-" + workerId;
            case TARGET_STATE_REQUEST -> this.message = "The worker with ID: " + workerId + " requested the Status: " + state.toString() + " to the gate with Gate-ID: " + gateId;
            default -> throw new IllegalStateException("Unexpected value: " + activityType);
        }
    }

    public GateActivityEntity() {

    }

    @Override
    public int compareTo(GateActivityEntity g) {
        if (this.equals(g)) {
            return 0;
        }
        int res = this.gateId.compareTo(g.getGateId());
        if (res == 0) {
            return this.gateTimeStamp.compareTo(g.getGateTimeStamp());
        }
        return res;
    }
}