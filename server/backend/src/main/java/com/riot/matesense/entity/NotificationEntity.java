package com.riot.matesense.entity;

import com.riot.matesense.enums.Status;
import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;


@Getter
@Setter
@Table(name = "notifications")
@Entity
public class NotificationEntity {
	@Getter
	@Setter
    @Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	private Long id;
	private Status status;
	private Timestamp lastTimeStamp;
	private Long workerId;
	private String message;


	public NotificationEntity(Status status, Long workerId, String message) {
		this.status = status;
		this.lastTimeStamp = new Timestamp(System.currentTimeMillis());
		this.workerId = workerId;
		this.message = message;
	}

	public NotificationEntity() {

	}
}
