package com.riot.matesense.repository;

import com.riot.matesense.entity.GateMetadataEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;
import java.util.Optional;

/**
 * A Repository for the GateMetadataEntity
 */
@Repository
public interface GateMetadataRepository extends JpaRepository<GateMetadataEntity, Long> {
	List<GateMetadataEntity> findByGateId(Long gateId);
	Optional<GateMetadataEntity> findByIdAndGateId(Long id, Long gateId);
	Optional<GateMetadataEntity> findByGateIdAndKey(Long gateId, String key);
	void deleteByGateId(Long gateId);
}
