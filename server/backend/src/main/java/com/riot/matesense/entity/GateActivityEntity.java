package com.riot.matesense.entity;

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
public class GateActivityEntity {
    @Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	private Long id;
	private Timestamp lastTimeStamp;
	private Long gateId;
	private String requestedStatus;
    private String message;
	private Long workerId;

	public GateActivityEntity(Timestamp lastTimeStamp, Long gateId, String requestedStatus, String message,  Long workerId) {

		this.lastTimeStamp = lastTimeStamp;
		this.gateId = gateId;
		this.requestedStatus = requestedStatus;
        this.message = message;
		this.workerId = workerId;
	}

	public GateActivityEntity() {

	}

}

