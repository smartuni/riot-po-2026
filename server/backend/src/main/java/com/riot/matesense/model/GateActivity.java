package com.riot.matesense.model;

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
	private String requestedStatus;
	private Long gateId;
	private Long workerId;
    @Getter
    private String message;

	public GateActivity(Timestamp lastTimeStamp, Long gateId, String requestedStatus, String message, Long id, Long workerId) {
        this.id = id;
		this.lastTimeStamp = lastTimeStamp;
		this.gateId = gateId;
		this.requestedStatus = requestedStatus;
        this.message = message;
		this.workerId = workerId;
	}
}
