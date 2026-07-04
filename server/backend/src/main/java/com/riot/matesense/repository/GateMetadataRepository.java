package com.riot.matesense.repository;

import com.riot.matesense.entity.GateMetadataEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

/**
 * A Repository for the GateMetadataEntity
 */
@Repository
public interface GateMetadataRepository extends JpaRepository<GateMetadataEntity, Long> {
	List<GateMetadataEntity> findByGateId(Long gateId);
}
