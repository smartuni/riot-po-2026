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
public class GateActivityEntity implements Comparable<GateActivityEntity>{
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

	public GateActivityEntity(Timestamp localTimeStamp, Timestamp gateTimeStamp, Long gateId, String requestedStatus, String message, Long workerId) {
		this.lastTimeStamp = localTimeStamp;
		this.localTimeStamp = localTimeStamp;
		this.gateTimeStamp = gateTimeStamp;
		this.gateId = gateId;
		this.requestedStatus = requestedStatus;
        this.message = message;
		this.workerId = workerId;
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

