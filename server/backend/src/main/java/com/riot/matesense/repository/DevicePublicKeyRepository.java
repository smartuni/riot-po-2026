package com.riot.matesense.repository;

import com.riot.matesense.entity.DevicePublicKeyEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.Optional;

@Repository
public interface DevicePublicKeyRepository extends JpaRepository<DevicePublicKeyEntity, Long> {
    Optional<DevicePublicKeyEntity> findByKid(String kid);
    boolean existsByKid(String kid);
    void deleteByKid(String kid);
}
