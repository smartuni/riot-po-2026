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
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;

@Service
public class NotificationService {

    @Autowired
    NotificationRepository notificationRepository;
    MsgType msgType;

    public List<Notification> getAllNotifications() {
        List<NotificationEntity> notificationEntities = notificationRepository.findAll();
        List<Notification> customNotifications = new ArrayList<>();
        notificationEntities.forEach(e -> {
            Notification notification = new Notification(e.getId(), e.getStatus(), e.getLastTimeStamp(), e.getWorkerId(), e.getMessage(), e.isRead());
            customNotifications.add(notification);
        });
        return customNotifications;
    }

    public String addNotification(NotificationEntity notification) throws GateAlreadyExistingException {
        notificationRepository.save(notification);
        return notification.toString();
    }


    public void removeNotification(NotificationEntity notification) throws GateNotFoundException {
        notificationRepository.delete(notification);
    }

    public List<Notification> getNotificationByWorkerId(Long id) {
        List<Notification> notifications = new ArrayList<>();
        for (NotificationEntity notification : notificationRepository.getByWorkerId(id)) {
            notifications.add(new Notification(notification.getId(), notification.getStatus(), notification.getLastTimeStamp(), notification.getWorkerId(), notification.getMessage(), notification.isRead()));
        }
        return notifications;
    }

    @Transactional
    public void requestNotificationReadChange(Long id, Boolean read) throws GateNotFoundException {
        NotificationEntity notification = notificationRepository.getById(id);
        System.out.println("NotificationService: requestNotificationReadChange: " + notification);
        notification.setRead(true);
        notificationRepository.save(notification);
        System.out.println(notificationRepository.getById(id).isRead());
    }


}
