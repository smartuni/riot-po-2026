package com.riot.matesense.entity;

import com.riot.matesense.enums.ConfidenceQuality;
import com.riot.matesense.enums.Status;
import com.riot.matesense.enums.MsgType;
import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

/**
 * An Entity for storing the Gates
 */
@Getter
@Setter
@Table(name = "gates")
@Entity
public class GateEntity {
	@Getter
	@Setter
    @Id
	//@GeneratedValue(strategy = GenerationType.IDENTITY)
	private Long id;
	private Status status;
	private Timestamp lastTimeStamp;
	private Long deviceId;
	private String location;
	private Double latitude;
	private Double longitude;
	private Boolean workerConfidence;
	private Boolean sensorConfidence;
	private ConfidenceQuality quality;
    private boolean ignoreGate;
    private boolean gateDetector;
    private Status[] gateStatusArray = new Status[3];
    private Status[] workerStatusArray = new Status[3];
	@Column(name = "requested_status")
	private String requestedStatus;
	private String pendingJob;
	private int confidence;
	private int priority = 0;


	public GateEntity(Long gateID,Status status, Timestamp lastTimeStamp,
					Double latitude, Double longitude,
					  String location, String requestedStatus,
					  int confidence,  String pendingJob, int priority) {
		this.id = gateID;
		this.status = status;
		this.lastTimeStamp = lastTimeStamp;
		// this.deviceId = deviceId;
		this.location = location;
		// this.workerConfidence = workerConfidence;
		// this.sensorConfidence = sensorConfidence;
		this.requestedStatus = requestedStatus;
		this.latitude = latitude;
		this.longitude = longitude;
		for(int i = 0; i < 3; i++)
		{
			this.gateStatusArray[i] = Status.NONE;
			this.workerStatusArray[i] = Status.NONE;
		}
		this.pendingJob = pendingJob;
		this.priority = priority;
	}

	public GateEntity() {

	}

	/**
	 * TODO://@Simon Korondi
	 * @param gateStatus
	 * @param reportType
	 */

	public void shuffleReports(Status gateStatus, MsgType reportType) // orders reports based on how recent they were
	{
		if(reportType == MsgType.IST_STATE) // if the report is from the gate's sensor
		{
			this.gateStatusArray[2] = this.gateStatusArray[1];
			this.gateStatusArray[1] = this.gateStatusArray[0]; // push older reports to the back of the array
			this.gateStatusArray[0] = gateStatus; // insert most recent report to the front of the array
		}
		else if(reportType == MsgType.SEEN_TABLE_STATE) // if the report is from a worker
		{
			this.workerStatusArray[2] = this.workerStatusArray[1];
			this.workerStatusArray[1] = this.workerStatusArray[0];
			this.workerStatusArray[0] = gateStatus;
		}
		else // update not originating from a gate or worker
		{
			return;
		}
	}
	
	//Uplink from SEEN_TABLE
	public GateEntity(Status status, Timestamp lastTimeStamp, Long deviceId) {
		this.status = status;
		this.lastTimeStamp = lastTimeStamp;
		this.deviceId = deviceId;
	}
	// HARD Coded
	public GateEntity(Status status, Timestamp lastTimeStamp,
					  Long deviceId, Double latitude, Double longitude,
					  String location, String requestedStatus){

		this.status = status;
		this.lastTimeStamp = lastTimeStamp;
		this.deviceId = deviceId;
		this.latitude = latitude;
		this.longitude = longitude;
		this.location = location;
		this.requestedStatus = requestedStatus;
	}
}
