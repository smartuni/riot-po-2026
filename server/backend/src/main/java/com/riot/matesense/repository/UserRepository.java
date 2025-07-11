package com.riot.matesense.repository;

import com.riot.matesense.entity.UserEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

/**
 * A Repository for the UserEntity
 */
@Repository
public interface UserRepository extends JpaRepository<UserEntity, Long> {
    UserEntity getById(Long id);
    UserEntity findByEmail(String email);
}
