package com.riot.matesense.model;

import com.riot.matesense.enums.Status;
import com.riot.matesense.enums.ConfidenceQuality;
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
	private int actualConfidence;
	private ConfidenceQuality quality;

	public Gate(Long id , Long deviceId, Timestamp lastTimeStamp, Status status, Double latitude, Double longitude,String location,
				 Boolean workerConfidence, Boolean sensorConfidence,
				String requestedStatus, int actualConfidence, ConfidenceQuality quality) {
		this.id = id;
		this.location = location;
		this.deviceId = deviceId;
		this.lastTimeStamp = lastTimeStamp;
		this.status = status;
		this.workerConfidence = workerConfidence;
		this.sensorConfidence = sensorConfidence;
		this.requestedStatus = requestedStatus;
		this.latitude = latitude;
		this.longitude = longitude;
		this.actualConfidence = actualConfidence;
		this.quality = quality;
	}
}
