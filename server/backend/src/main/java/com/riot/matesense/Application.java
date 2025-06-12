package com.riot.matesense;

import com.riot.matesense.datatypes.Coordinate;
import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.Status;
import com.riot.matesense.model.Gate;
import com.riot.matesense.repository.GateRepository;
import com.riot.matesense.service.GateService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.domain.EntityScan;
import org.springframework.stereotype.Component;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.List;

@SpringBootApplication
@EntityScan(basePackages = "com.riot.matesense.entity")
public class Application {

	public static void main(String[] args) {
		SpringApplication.run(Application.class, args);
	}

}

@Component
class PopulateTestDataRunner implements CommandLineRunner {
	GateService gateService;
	GateRepository gateRepository;
	@Autowired
	public PopulateTestDataRunner(GateService gateService, GateRepository gateRepository) {
		this.gateService = gateService;
		this.gateRepository = gateRepository;
	}
	@Override
	public void run(String... args) throws Exception {
		List <Coordinate> coordinates = new ArrayList<>();
		Coordinate coordinate = new Coordinate(23.000, 23.4444);
		coordinates.add(coordinate);
		GateEntity gate = new GateEntity(Status.CLOSED, new Timestamp(System.currentTimeMillis()), 1L, 53.540808,9.9654, "St.Pauli", false, true, null);
//		GateEntity gate1 = new GateEntity(Status.CLOSED, new Timestamp(System.currentTimeMillis()), 2L, coordinates, "Landungsbrücken", false, true, null);
//		GateEntity gate2 = new GateEntity(Status.OPEN, new Timestamp(System.currentTimeMillis()), 3L, coordinates, "Veddel", true, false, null);
//		GateEntity gate3 = new GateEntity(Status.OPEN, new Timestamp(System.currentTimeMillis()), 4L, coordinates, "Hafen", true, true, null);
//		gateRepository.save(gate);
//		gateRepository.save(gate1);
//		GateEntity gate = new GateEntity(Status.CLOSED, new Timestamp(System.currentTimeMillis()), 1L, 53.5408, 9.9654, "St.Pauli", false, true, "OPEN");
//		GateEntity gate1 = new GateEntity(Status.CLOSED, new Timestamp(System.currentTimeMillis()), 2L, 53.5409, 9.8674, "Landungsbrücken", false, true, null);
//		GateEntity gate2 = new GateEntity(Status.OPEN, new Timestamp(System.currentTimeMillis()), 3L, 53.5410, 9.8664, "Veddel", true, false, null);
//		GateEntity gate3 = new GateEntity(Status.OPEN, new Timestamp(System.currentTimeMillis()), 4L, 53.5460, 9.8634, "Hafen", true, true, "CLOSED");
//
//		gateRepository.save(gate);
//		gateRepository.save(gate1);
//
//
//		gateService.addGate(gate);
//		gateService.addGate(gate1);
//		gateService.addGate(gate2);
//		gateService.addGate(gate3);
	}
}