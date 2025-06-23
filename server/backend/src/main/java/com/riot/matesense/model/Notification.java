package com.riot.matesense.model;

import com.riot.matesense.enums.Status;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

@Setter
@Getter
public class Notification {
	private Long id;
	private Status status;
	private Timestamp lastTimeStamp;
	private Long workerId;
	private String message;
	private boolean read;

	public Notification(Long id, Status status, Timestamp lastTimeStamp, Long workerId, String message, boolean read) {
		this.id = id;
		this.status = status;
		this.lastTimeStamp = lastTimeStamp;
		this.workerId = workerId;
		this.message = message;
		this.read = read;
	}
}
