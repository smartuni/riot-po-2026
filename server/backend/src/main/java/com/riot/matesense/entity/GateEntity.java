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


	public GateEntity(Status status, Timestamp lastTimeStamp,
					  Long deviceId, Double latitude, Double longitude,
					  String location, Boolean workerConfidence,
					  Boolean sensorConfidence, String requestedStatus) {
		this.status = status;
		this.lastTimeStamp = lastTimeStamp;
		this.deviceId = deviceId;
		this.location = location;
		this.workerConfidence = workerConfidence;
		this.sensorConfidence = sensorConfidence;
		this.requestedStatus = requestedStatus;
		this.latitude = latitude;
		this.longitude = longitude;
	}

	public GateEntity() {

	}
}
