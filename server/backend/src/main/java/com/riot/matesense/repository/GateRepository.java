package com.riot.matesense.repository;

import com.riot.matesense.entity.GateEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface GateRepository extends JpaRepository<GateEntity, Integer> {
	GateEntity getById(Long id);
}
