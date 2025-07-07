package com.riot.matesense.model;

import com.riot.matesense.enums.Status;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

@Setter
@Getter
public class Gate {
	@Getter
	private Long id;
	private Status status;
	private Timestamp lastTimeStamp;
	private Double latitude;
	private Double longitude;
	private Long deviceId;
	private String location;
	private Boolean workerConfidence;
	private Boolean sensorConfidence;
	private String requestedStatus;
	private String pendingJob;
	private int confidence;
	private int priority;

	public Gate(Long id , Long deviceId, Timestamp lastTimeStamp, Status status,
				Double latitude, Double longitude,String location,
				 Boolean workerConfidence, Boolean sensorConfidence,
				String requestedStatus,  int confidence,
				String pendingJob, int priority) {
		this.id = id;
		this.location = location;
		this.deviceId = deviceId;
		this.lastTimeStamp = lastTimeStamp;
		this.status = status;
		this.workerConfidence = workerConfidence;
		this.sensorConfidence = sensorConfidence;
		this.requestedStatus = requestedStatus;
		this.pendingJob = pendingJob;
		this.latitude = latitude;
		this.longitude = longitude;
		this.confidence = confidence;
		this.priority = priority;
	}
}
