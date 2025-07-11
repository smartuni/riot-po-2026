package com.riot.matesense.service;

import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.entity.NotificationEntity;
import com.riot.matesense.enums.MsgType;
import com.riot.matesense.exceptions.GateAlreadyExistingException;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.model.Gate;
import com.riot.matesense.model.GateForDownlink;
import com.riot.matesense.model.Notification;
import com.riot.matesense.repository.GateRepository;
import com.riot.matesense.repository.NotificationRepository;
import jakarta.transaction.Transactional;
import org.aspectj.weaver.ast.Not;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;

@Service
public class NotificationService {
    /**
     * a fassade class for the service methods for the Notifications
     */
    @Autowired
    NotificationRepository notificationRepository;
    private SimpMessagingTemplate messagingTemplate;
    MsgType msgType;

    public NotificationService(NotificationRepository notificationRepository,
                               SimpMessagingTemplate messagingTemplate) {
        this.notificationRepository = notificationRepository;
        this.messagingTemplate = messagingTemplate;
    }

    /**
     * a method that gets all the Notifications from the db
     * @return a list with all the notifications
     */
    public List<Notification> getAllNotifications() {
        List<NotificationEntity> notificationEntities = notificationRepository.findAll();
        List<Notification> customNotifications = new ArrayList<>();
        notificationEntities.forEach(e -> {
            Notification notification = new Notification(e.getId(), e.getStatus(), e.getLastTimeStamp(), e.getWorkerId(), e.getMessage(), e.isRead());
            customNotifications.add(notification);
        });
        return customNotifications;
    }

    /**
     * a method to add a notification to the db
     * @param notification to be added
     * @return notification as a String
     * @throws GateAlreadyExistingException
     */
    public String addNotification(NotificationEntity notification) throws GateAlreadyExistingException {
        NotificationEntity created = notificationRepository.save(notification);
        messagingTemplate.convertAndSend("/topic/notifications", created);
        return notification.toString();
    }

    /**
     * a method for deleting a notification
     * @param notification that should be deleted
     * @throws GateNotFoundException
     */
    public void removeNotification(NotificationEntity notification) throws GateNotFoundException {
        notificationRepository.delete(notification);
        messagingTemplate.convertAndSend("/topic/notifications/delete", notification.getId());
    }

    /**
     * a method to get a notification for a worker by its id
     * @param id of the worker
     * @return the notifications to the worker
     */
    public List<Notification> getNotificationByWorkerId(Long id) {
        List<Notification> notifications = new ArrayList<>();
        for (NotificationEntity notification : notificationRepository.getByWorkerId(id)) {
            notifications.add(new Notification(notification.getId(), notification.getStatus(), notification.getLastTimeStamp(), notification.getWorkerId(), notification.getMessage(), notification.isRead()));
        }
        return notifications;
    }

    /**
     * a method to change the status of a notification to read
     * @param id of the notifications
     * @param read the boolean
     * @throws GateNotFoundException
     */
    @Transactional
    public void requestNotificationReadChange(Long id, Boolean read) throws GateNotFoundException {
        NotificationEntity notification = notificationRepository.getById(id);
        System.out.println("NotificationService: requestNotificationReadChange: " + notification);
        notification.setRead(true);
        notificationRepository.save(notification);
        messagingTemplate.convertAndSend("/topic/notifications/read-change", notification);
        System.out.println(notificationRepository.getById(id).isRead());
    }


}
