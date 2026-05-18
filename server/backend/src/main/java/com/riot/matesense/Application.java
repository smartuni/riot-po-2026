package com.riot.matesense;

import com.riot.matesense.entity.DownlinkCounterEntity;
import com.riot.matesense.entity.NotificationEntity;
import com.riot.matesense.enums.Status;
import com.riot.matesense.repository.DownlinkCounterRepository;
import com.riot.matesense.repository.GateRepository;
import com.riot.matesense.service.DownlinkService;
import com.riot.matesense.service.GateActivityService;
import com.riot.matesense.service.GateService;
import com.riot.matesense.service.NotificationService;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.domain.EntityScan;
import org.springframework.context.annotation.Profile;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.stereotype.Component;

@SpringBootApplication
@EnableScheduling
@EntityScan(basePackages = "com.riot.matesense.entity")
public class Application {

	public static void main(String[] args) {
		SpringApplication.run(Application.class, args);
	}

}

@Component
@Profile("dev") // Only active in development profile
class PopulateTestDataRunner implements CommandLineRunner {

	private final DownlinkCounterRepository downlinkCounterRepository;
	DownlinkService downlinkService;
	GateService gateService;
	GateRepository gateRepository;
	GateActivityService gateActivityService;
	NotificationService notificationService;
	
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
		// Notifications disabled - they cause DB enum errors
		// These will be added via Flyway migration in the future
		/*
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
		*/
		
		DownlinkCounterEntity downlinkCounterEntity = new DownlinkCounterEntity();
		downlinkCounterRepository.save(downlinkCounterEntity);
	}
}