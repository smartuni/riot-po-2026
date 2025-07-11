package com.riot.matesense.repository;

import com.riot.matesense.entity.GateActivityEntity;
import com.riot.matesense.entity.GateEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

/**
 * A Repository for the GateActivityEntity
 */
@Repository
public interface GateActivityRepository extends JpaRepository<GateActivityEntity, Integer> {
	GateActivityEntity getById(Long id);
}

