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
@RequestMapping("/notifications")
public class NotificationController {
    /**
     * A Controller for the Downlink-Counter
     * A Downlink-Counter is a counter that is stored in the database which tracks the sent downlinks
     */
    @Autowired
    private NotificationService notificationService;

    /**
     * AN API Call to get all the notifications from the database
     * @return a list of the notifications
     */
    @GetMapping
    public List<Notification> getAllNotifications() {
        return notificationService.getAllNotifications();
    }

    /**
     * AN API Call to add a Notification
     * @param notification that should be added
     * @return thee notification that has been added
     * @throws GateAlreadyExistingException
     */
    @PostMapping("/add")
    public String addNotification(@RequestBody NotificationEntity notification)
            throws GateAlreadyExistingException {
        return notificationService.addNotification(notification);
    }

    /**
     * AN API Call to delete a Notification
     * @param notification the notification that should be deleted
     * @throws GateNotFoundException
     */
    @DeleteMapping("/delete")
    public void removeNotification(@RequestBody NotificationEntity notification)
            throws GateNotFoundException {
        notificationService.removeNotification(notification);
    }

    /**
     * AN API Call that gets all the notifications from a worker by their worker ID
     * @param workerId from the worker
     * @return all the notifications from a worker in a list
     */
    @GetMapping("/{workerId}")
    public List<Notification> getAllNotificationsByWorkerId(@PathVariable Long workerId) {
        return notificationService.getNotificationByWorkerId(workerId);
    }

    /**
     * AN API Call to set a notifications a read
     * @param notificationId of the notification
     * @throws GateNotFoundException
     */
    @PostMapping("/{notificationId}/request-read-change")
    public void requestNotificationReadChange(@PathVariable Long notificationId)
            throws GateNotFoundException {
        notificationService.requestNotificationReadChange(notificationId, true);
    }
}
