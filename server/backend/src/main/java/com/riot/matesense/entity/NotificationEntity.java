package com.riot.matesense.entity;

import com.riot.matesense.enums.Status;
import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

/**
 * An Entity for storing Notifications that are created when stuff happens to the Gates
 */
@Getter
@Setter
@Table(name = "notifications")
@Entity
public class NotificationEntity {
    @Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	private Long id;
	private Status status;
	private Timestamp lastTimeStamp;
	private Long workerId;
	private String message;
	@Getter
	private boolean read;


	public NotificationEntity(Status status, Long workerId, String message, boolean read) {
		this.status = status;
		this.lastTimeStamp = new Timestamp(System.currentTimeMillis());
		this.workerId = workerId;
		this.message = message;
		this.read = read; // Default value for read status
	}

	public NotificationEntity() {

	}

	public boolean isRead() {
		return read;
	}

	public void setRead(boolean read) {
		this.read = read;
	}
}
