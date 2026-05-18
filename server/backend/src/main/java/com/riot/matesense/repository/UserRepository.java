package com.riot.matesense.repository;

import com.riot.matesense.entity.UserEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;
import java.util.Optional;

/**
 * A Repository for the UserEntity
 */
@Repository
public interface UserRepository extends JpaRepository<UserEntity, Long> {
    /**
     * Find a user by their email address
     * @param email the email to search for
     * @return UserEntity if found, null otherwise
     */
    UserEntity findByEmail(String email);

    /**
     * Find a user by their ID with Optional support
     * @param id the user ID
     * @return Optional containing the UserEntity if found
     */
    Optional<UserEntity> findById(Long id);
}
