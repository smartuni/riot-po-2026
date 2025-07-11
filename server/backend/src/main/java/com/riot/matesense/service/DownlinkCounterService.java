package com.riot.matesense.service;

import com.riot.matesense.entity.DownlinkCounterEntity;
import com.riot.matesense.entity.NotificationEntity;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.DownlinkCounter;
import com.riot.matesense.model.Notification;
import com.riot.matesense.repository.DownlinkCounterRepository;
import com.riot.matesense.repository.NotificationRepository;
import jakarta.transaction.Transactional;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Optional;

@Service
public class DownlinkCounterService {

    private final DownlinkCounterRepository repository;

    @Autowired
    public DownlinkCounterService(DownlinkCounterRepository repository) {
        this.repository = repository;
    }

    public int getCounter() {
        DownlinkCounterEntity counterOpt = repository.getById(1L);
        return counterOpt.getCounter();
    }

    public DownlinkCounterEntity getCounterById() {
        return repository.getById(1L);
    }

    public boolean incrementCounterIfBelowLimit() {
        DownlinkCounterEntity counter = repository.getById(1L);

        if (counter.getCounter() >= 10) {
            return false;
        }

        counter.setCounter(counter.getCounter() + 1);
        repository.save(counter);
        return true;
    }

    // Optional: Zurücksetzen (z. B. einmal pro Tag durch Admin oder Cron)
    public void resetCounter() {
        DownlinkCounterEntity counter = repository.getById(1L);
        if (counter != null) {
            counter.setCounter(0);
            repository.save(counter);
        }
    }

    @Scheduled(cron = "0 0 0 * * *") // jeden Tag um 00:00 Uhr
    public void scheduledReset() {
        resetCounter();
        System.out.println("Downlink counter wurde um Mitternacht zurückgesetzt.");
    }
}

