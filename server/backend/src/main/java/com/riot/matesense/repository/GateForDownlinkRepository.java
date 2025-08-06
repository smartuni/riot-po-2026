
package com.riot.matesense.repository;

import com.riot.matesense.entity.GateForDownlinkEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

/**
 * A Repository for the GateForDownlinkEntity
 */
@Repository
public interface GateForDownlinkRepository extends JpaRepository<GateForDownlinkEntity, Integer> {
    GateForDownlinkEntity getById(Long id);
}

