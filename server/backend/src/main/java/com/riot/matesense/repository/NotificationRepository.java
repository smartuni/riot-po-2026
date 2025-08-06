package com.riot.matesense.repository;

import com.riot.matesense.entity.NotificationEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface NotificationRepository extends JpaRepository<NotificationEntity, Integer> {
	NotificationEntity getById(Long id);
	List<NotificationEntity> getByWorkerId(Long workerId);
}
