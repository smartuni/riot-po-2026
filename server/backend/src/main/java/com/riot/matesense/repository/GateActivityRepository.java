package com.riot.matesense.repository;

import com.riot.matesense.entity.GateActivityEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

/**
 * A Repository for the GateActivityEntity
 */
@Repository
public interface GateActivityRepository extends JpaRepository<GateActivityEntity, Integer> {
	GateActivityEntity getById(Long id);
	List<WorkerIdView> findDistinctWorkerIdByGateId(Long gateId);
}

