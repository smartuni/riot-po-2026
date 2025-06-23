package com.riot.matesense.model;

import com.riot.matesense.enums.Status;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

@Setter
@Getter
public class Notification {
	@Getter
	private Long id;
	private Status status;
	private Timestamp lastTimeStamp;
	private Long workerId;
	private String message;

	public Notification(Long id, Status status, Timestamp lastTimeStamp, Long workerId, String message) {
		this.id = id;
		this.status = status;
		this.lastTimeStamp = lastTimeStamp;
		this.workerId = workerId;
		this.message = message;
	}
}
