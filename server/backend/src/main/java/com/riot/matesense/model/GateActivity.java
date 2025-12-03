package com.riot.matesense.model;

import com.riot.matesense.enums.ActivityType;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

/**
 * A DTO for the GateActivityEntity
 */
@Setter
@Getter
public class GateActivity {
	private Long id;
	private Timestamp lastTimeStamp;
	private Timestamp localTimeStamp;
	private Timestamp gateTimeStamp;
	private String requestedStatus;
	private Long gateId;
	private Long workerId;
    @Getter
    private String message;
    private ActivityType activityType;

	public GateActivity(Timestamp localTimeStamp, Timestamp gateTimeStamp, Long gateId, String requestedStatus,
                        String message, Long id, Long workerId, ActivityType activityType) {
        this.id = id;
        this.lastTimeStamp = localTimeStamp;
        this.localTimeStamp = localTimeStamp;
        this.gateTimeStamp = gateTimeStamp;
		this.gateId = gateId;
		this.requestedStatus = requestedStatus;
        this.message = message;
		this.workerId = workerId;
		this.activityType = activityType;
	}
}
