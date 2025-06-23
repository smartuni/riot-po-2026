package com.riot.matesense.controller;

import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.entity.NotificationEntity;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.Gate;
import com.riot.matesense.model.GateForDownlink;
import com.riot.matesense.model.Notification;
import com.riot.matesense.service.GateService;
import com.riot.matesense.service.NotificationService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;


@RestController
// @CrossOrigin(origins = "*")
public class NotificationController {

    @Autowired
    NotificationService notificationService;

    @RequestMapping(value = "notifications", method = RequestMethod.GET)
    public List<Notification> getAllNotifications() {
        return notificationService.getAllNotifications();
    }

    @RequestMapping(value = "add-notification", method = RequestMethod.POST)
    public String addNotification(@RequestBody NotificationEntity notification) throws GateAlreadyExistingException {
        return notificationService.addNotification(notification);
    }

    @RequestMapping(value = "delete-notification", method = RequestMethod.DELETE)
    public void removeNotification(@RequestBody NotificationEntity notification) throws GateNotFoundException {
        notificationService.removeNotification(notification);
    }
}
