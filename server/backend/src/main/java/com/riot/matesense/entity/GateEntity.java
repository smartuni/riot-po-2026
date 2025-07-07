package com.riot.matesense.entity;

import com.riot.matesense.enums.Status;
import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;


@Getter
@Setter
@Table(name = "gates")
@Entity
public class GateEntity {
	@Getter
	@Setter
    @Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	private Long id;
	private Status status;
	private Timestamp lastTimeStamp;
	private Long deviceId;
	private String location;
	private Double latitude;
	private Double longitude;
	private Boolean workerConfidence;
	private Boolean sensorConfidence;
	@Column(name = "requested_status")
	private String requestedStatus;
	private String pendingJob;
	private int confidence;
	private int priority = 0;


	public GateEntity(Status status, Timestamp lastTimeStamp,
					Double latitude, Double longitude,
					  String location, String requestedStatus,
					  int confidence,  String pendingJob, int priority) {
		this.status = status;
		this.lastTimeStamp = lastTimeStamp;
		// this.deviceId = deviceId;
		this.location = location;
		// this.workerConfidence = workerConfidence;
		// this.sensorConfidence = sensorConfidence;
		this.requestedStatus = requestedStatus;
		this.latitude = latitude;
		this.longitude = longitude;
		this.confidence = confidence;
		this.pendingJob = pendingJob;
		this.priority = priority;
	}

	public GateEntity() {

	}
}
