package com.riot.matesense;

import com.riot.matesense.config.DownPayload;

import com.riot.matesense.entity.DownlinkCounterEntity;
import com.riot.matesense.entity.GateActivityEntity;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.entity.NotificationEntity;
import com.riot.matesense.enums.Status;
import com.riot.matesense.repository.DownlinkCounterRepository;
import com.riot.matesense.repository.GateRepository;
import com.riot.matesense.service.DownlinkService;
import com.riot.matesense.service.GateActivityService;
import com.riot.matesense.service.GateService;
import com.riot.matesense.service.NotificationService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.domain.EntityScan;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.stereotype.Component;

import java.sql.Timestamp;
import java.util.Arrays;

@SpringBootApplication
@EnableScheduling
@EntityScan(basePackages = "com.riot.matesense.entity")
public class Application {

	public static void main(String[] args) {
		SpringApplication.run(Application.class, args);
	}

}

@Component
class PopulateTestDataRunner implements CommandLineRunner {

	private final DownlinkCounterRepository downlinkCounterRepository;
	DownlinkService downlinkService;
	GateService gateService;
	GateRepository gateRepository;
	GateActivityService gateActivityService;
	NotificationService notificationService;

	@Autowired
	public PopulateTestDataRunner(GateActivityService gateActivityService, GateService gateService, GateRepository gateRepository,
								  DownlinkService downlinkService, NotificationService notificationService, DownlinkCounterRepository downlinkCounterRepository) {
		this.gateService = gateService;
		this.gateRepository = gateRepository;
		this.downlinkService = downlinkService;
		this.gateActivityService = gateActivityService;
		this.notificationService = notificationService;
		this.downlinkCounterRepository = downlinkCounterRepository;
	}
	@Override
	public void run(String... args) throws Exception {
		//DownPayload downPayload =new DownPayload(1,247,Arrays.asList(Arrays.asList(1,0,1),Arrays.asList(2,1,0)));
		//[1,247,[[187,0],[69,1]]]
		// Beispiel für ein größeres DownPayload mit 6 Einträgen
//		DownPayload downPayload = new DownPayload(
//				1, // Message Type
//				247, // Globaler Timestamp
//				Arrays.asList(
//						Arrays.asList(1, 0, 3), // GateID 101, Soll-Status 0, Prio 3
//						Arrays.asList(2, 1, 2), // GateID 102, Soll-Status 1, Prio 2
//						Arrays.asList(3, 0, 1), // GateID 103, Soll-Status 0, Prio 1
//						Arrays.asList(4, 1, 3), // GateID 104, Soll-Status 1, Prio 3
//						Arrays.asList(5, 0, 2), // GateID 105, Soll-Status 0, Prio 2
//						Arrays.asList(6, 1, 1)  // GateID 106, Soll-Status 1, Prio 1
//				)
//		);
/*
		//downlinkService.sendDownlinkToDevice(downPayload); // Test call
 		GateEntity gate = new GateEntity(1L, Status.OPENED, new Timestamp(System.currentTimeMillis()), 53.5400, 9.8700, "Hauptbahnhof", "REQUESTED_NONE", 0,  "PENDING_NONE", 3);
 		//GateEntity gate1 = new GateEntity(2L, Status.UNKNOWN, new Timestamp(System.currentTimeMillis()), 53.5409, 9.8674, "Landungsbrücken", "REQUESTED_NONE", 0, "PENDING_NONE", 3);
 		GateEntity gate2 = new GateEntity(3L, Status.OPENED, new Timestamp(System.currentTimeMillis()), 53.5410, 9.8664, "Veddel", "REQUESTED_NONE", 0,"PENDING_NONE", 3);
 		GateEntity gate3 = new GateEntity(4L, Status.CLOSED, new Timestamp(System.currentTimeMillis()), 53.5430, 9.8434, "Hafen",  "REQUESTED_NONE",0, "PENDING_NONE", 3);
 		GateEntity gate4 = new GateEntity(5L, Status.OPENED, new Timestamp(System.currentTimeMillis()), 53.5440, 9.8734, "Steinwerder",  "REQUESTED_NONE",0, "PENDING_NONE", 3);
 		GateEntity gate5 = new GateEntity(6L, Status.CLOSED, new Timestamp(System.currentTimeMillis()), 53.5470, 9.8634, "Neumühlen",  "REQUESTED_NONE",0, "PENDING_NONE", 3);
 		gateService.addGate(gate);
 		//gateService.addGate(gate1);
 		gateService.addGate(gate2);
 		gateService.addGate(gate3);
 		gateService.addGate(gate4);
 		gateService.addGate(gate5);
*/

		notificationService.addNotification(new NotificationEntity(Status.OPENED, 1L, "Worker with ID: " + 1L + " shall close the Gate with ID: " + 1, false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 1L, "Worker with ID: " + 1L + " shall close the Gate with ID: " + 1, false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 2L, "Worker with ID: " + 2L + " shall close the Gate with ID: " + 1, false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 2L, "Worker with ID: " + 2L + " shall close the Gate with ID: " + 1, false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 3L, "Worker with ID: " + 3L + " shall close the Gate with ID: " + 1, false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 3L, "Worker with ID: " + 3L + " shall close the Gate with ID: " + 1, false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 4L, "Worker with ID: " + 4L + " shall close the Gate with ID: " + 1, false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 4L, "Worker with ID: " + 4L + " shall close the Gate with ID: " + 1, false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 5L, "Worker with ID: " + 5L + " shall close the Gate with ID: " + 1, false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 5L, "Worker with ID: " + 5L + " shall close the Gate with ID: " + 1, false));
		DownlinkCounterEntity downlinkCounterEntity = new DownlinkCounterEntity();
		downlinkCounterRepository.save(downlinkCounterEntity);
	}
}