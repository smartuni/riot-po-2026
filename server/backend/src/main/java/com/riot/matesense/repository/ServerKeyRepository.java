package com.riot.matesense.repository;

import com.riot.matesense.entity.ServerKeyEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface ServerKeyRepository extends JpaRepository<ServerKeyEntity, Long> {
}
