package com.riot.matesense.repository;

import com.riot.matesense.entity.UserEntity;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.orm.jpa.DataJpaTest;
import org.springframework.test.context.ActiveProfiles;

import static org.assertj.core.api.Assertions.assertThat;

@DataJpaTest
@ActiveProfiles("test")
class UserRepositoryTest {

    @Autowired
    private UserRepository userRepository;

    @Test
    void savesUserInDatabase() {
        UserEntity user = new UserEntity(
                "junit@example.com",
                "secret",
                "JUnit User",
                "viewer"
        );

        UserEntity savedUser = userRepository.save(user);

        assertThat(savedUser.getId()).isNotNull();
        assertThat(userRepository.findByEmail("junit@example.com"))
                .isNotNull()
                .extracting(UserEntity::getName)
                .isEqualTo("JUnit User");
    }
}
