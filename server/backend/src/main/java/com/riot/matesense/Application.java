package com.riot.matesense;

import com.riot.matesense.config.DownPayload;

import com.riot.matesense.entity.GateActivityEntity;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.entity.NotificationEntity;
import com.riot.matesense.enums.Status;
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
import org.springframework.stereotype.Component;

import java.sql.Timestamp;
import java.util.Arrays;

@SpringBootApplication
@EntityScan(basePackages = "com.riot.matesense.entity")
public class Application {

	public static void main(String[] args) {
		SpringApplication.run(Application.class, args);
	}

}

@Component
class PopulateTestDataRunner implements CommandLineRunner {

	DownlinkService downlinkService;
	GateService gateService;
	GateRepository gateRepository;
	GateActivityService gateActivityService;
	NotificationService notificationService;

	@Autowired
	public PopulateTestDataRunner(GateActivityService gateActivityService, GateService gateService, GateRepository gateRepository,
								  DownlinkService downlinkService, NotificationService notificationService) {
		this.gateService = gateService;
		this.gateRepository = gateRepository;
		this.downlinkService = downlinkService;
		this.gateActivityService = gateActivityService;
		this.notificationService = notificationService;
	}
	@Override
	public void run(String... args) throws Exception {
		DownPayload downPayload =new DownPayload(1,247,Arrays.asList(Arrays.asList(1,0),Arrays.asList(2,1)));
		//[1,247,[[187,0],[69,1]]]
		downlinkService.sendDownlinkToDevice(downPayload); // Test call
 		GateEntity gate = new GateEntity(Status.CLOSED, new Timestamp(System.currentTimeMillis()), 1L, 53.5408, 9.9654, "St.Pauli", false, true, "REQUESTED_OPEN", "100%", "PENDING_OPEN", 1);
 		GateEntity gate1 = new GateEntity(Status.CLOSED, new Timestamp(System.currentTimeMillis()), 2L, 53.5409, 9.8674, "Landungsbrücken", false, true, "REQUESTED_OPEN", "75%", "PENDING_OPEN", 2);
 		GateEntity gate2 = new GateEntity(Status.OPENED, new Timestamp(System.currentTimeMillis()), 3L, 53.5410, 9.8664, "Veddel", true, false, "REQUESTED_CLOSE",  "80%", "PENDING_CLOSE", 3);
 		GateEntity gate3 = new GateEntity(Status.OPENED, new Timestamp(System.currentTimeMillis()), 4L, 53.5460, 9.8634, "Hafen", true, true, "REQUESTED_NONE", "60%", "PENDING_NONE", 0);
 		gateService.addGate(gate);
 		gateService.addGate(gate1);
 		gateService.addGate(gate2);
 		gateService.addGate(gate3);


/*
		GateActivityEntity gateActivityEntity1 = new GateActivityEntity(new Timestamp(System.currentTimeMillis()), gate.getId(), gate.getRequestedStatus(), "The Gate " +gate1.getId() +" has Requested Status: " + gate.getRequestedStatus().toLowerCase(), 1L);
		GateActivityEntity gateActivityEntity2 = new GateActivityEntity(new Timestamp(System.currentTimeMillis()), gate1.getId(), gate.getRequestedStatus(), "The Gate " +gate2.getId() +" has Requested Status: " + gate.getRequestedStatus().toLowerCase(), 1L);
		GateActivityEntity gateActivityEntity3 = new GateActivityEntity(new Timestamp(System.currentTimeMillis()), gate2.getId(), gate.getRequestedStatus(), "The Gate " +gate3.getId() +" has Requested Status: " + gate.getRequestedStatus().toLowerCase(), 1L);
		GateActivityEntity gateActivityEntity4 = new GateActivityEntity(new Timestamp(System.currentTimeMillis()), gate3.getId(), gate.getRequestedStatus(), "The Gate " +gate.getId() +" has Requested Status: " + gate.getRequestedStatus().toLowerCase(), 1L);
		GateActivityEntity gateActivityEntity5 = new GateActivityEntity(new Timestamp(System.currentTimeMillis()), gate.getId(), gate.getRequestedStatus(), "The Gate " +gate1.getId() +" has Requested Status: " + gate.getRequestedStatus().toLowerCase(), 1L);

		gateActivityService.addGateActivity(gateActivityEntity1);
		gateActivityService.addGateActivity(gateActivityEntity2);
		gateActivityService.addGateActivity(gateActivityEntity3);
		gateActivityService.addGateActivity(gateActivityEntity4);
		gateActivityService.addGateActivity(gateActivityEntity5);
*/

		notificationService.addNotification(new NotificationEntity(Status.OPENED, 1L, "Worker with ID: " + 1L + " shall close the Gate with ID: " + gate.getId(), false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 1L, "Worker with ID: " + 1L + " shall open the Gate with ID: " + gate.getId(), false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 2L, "Worker with ID: " + 2L + " shall close the Gate with ID: " + gate.getId(), false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 2L, "Worker with ID: " + 2L + " shall close the Gate with ID: " + gate.getId(), false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 3L, "Worker with ID: " + 3L + " shall close the Gate with ID: " + gate.getId(), false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 3L, "Worker with ID: " + 3L + " shall close the Gate with ID: " + gate.getId(), false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 4L, "Worker with ID: " + 4L + " shall close the Gate with ID: " + gate.getId(), false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 4L, "Worker with ID: " + 4L + " shall close the Gate with ID: " + gate.getId(), false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 5L, "Worker with ID: " + 5L + " shall close the Gate with ID: " + gate.getId(), false));
		notificationService.addNotification(new NotificationEntity(Status.OPENED, 5L, "Worker with ID: " + 5L + " shall close the Gate with ID: " + gate.getId(), false));

		//test commit
	}
}