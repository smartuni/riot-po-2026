package com.riot.matesense.repository;

import com.riot.matesense.entity.DownlinkCounterEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

/**
 * A Repository for the DownlinkCounterEntity
 */
@Repository
public interface DownlinkCounterRepository extends JpaRepository<DownlinkCounterEntity, Integer> {
	DownlinkCounterEntity getById(Long id);
}
