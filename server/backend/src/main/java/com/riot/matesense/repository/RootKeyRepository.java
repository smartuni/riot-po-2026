package com.riot.matesense.repository;

import com.riot.matesense.entity.RootKeyEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface RootKeyRepository extends JpaRepository<RootKeyEntity, Long> {
}
