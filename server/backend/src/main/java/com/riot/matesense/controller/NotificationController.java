package com.riot.matesense.controller;

import com.riot.matesense.entity.NotificationEntity;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.Notification;
import com.riot.matesense.service.NotificationService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/notifications")  // zentraler Pfad
public class NotificationController {

    @Autowired
    private NotificationService notificationService;

    // GET /notifications
    @GetMapping
    public List<Notification> getAllNotifications() {
        return notificationService.getAllNotifications();
    }

    // POST /notifications/add
    @PostMapping("/add")
    public String addNotification(@RequestBody NotificationEntity notification)
            throws GateAlreadyExistingException {
        return notificationService.addNotification(notification);
    }

    // DELETE /notifications/delete
    @DeleteMapping("/delete")
    public void removeNotification(@RequestBody NotificationEntity notification)
            throws GateNotFoundException {
        notificationService.removeNotification(notification);
    }

    // GET /notifications/{workerId}
    @GetMapping("/{workerId}")
    public List<Notification> getAllNotificationsByWorkerId(@PathVariable Long workerId) {
        return notificationService.getNotificationByWorkerId(workerId);
    }

    // POST /notifications/{notificationId}/request-read-change
    @PostMapping("/{notificationId}/request-read-change")
    public void requestNotificationReadChange(@PathVariable Long notificationId)
            throws GateNotFoundException {
        notificationService.requestNotificationReadChange(notificationId, true);
    }
}
