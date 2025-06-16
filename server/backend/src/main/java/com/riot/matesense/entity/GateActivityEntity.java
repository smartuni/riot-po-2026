package com.riot.matesense.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;


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

	public GateActivityEntity(Timestamp lastTimeStamp, Long gateId, String requestedStatus, String message) {

		this.lastTimeStamp = lastTimeStamp;
		this.gateId = gateId;
		this.requestedStatus = requestedStatus;
        this.message = message;
	}

	public GateActivityEntity() {

	}
}

