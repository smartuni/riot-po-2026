package com.riot.matesense.repository;

import com.riot.matesense.entity.RootKeyEntity;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.orm.jpa.DataJpaTest;
import org.springframework.test.context.ActiveProfiles;

import java.util.List;

import static org.assertj.core.api.Assertions.assertThat;

@DataJpaTest
@ActiveProfiles("test")
class RootKeyRepositoryTest {

    @Autowired
    private RootKeyRepository rootKeyRepository;

    @Test
    void savesRootKeyInDatabase() {
        RootKeyEntity key = new RootKeyEntity();
        key.setPublicKey("pub-key-123");
        key.setPrivateKey("priv-key-456");

        RootKeyEntity saved = rootKeyRepository.save(key);

        assertThat(saved.getId()).isNotNull();
        assertThat(saved.getPublicKey()).isEqualTo("pub-key-123");
        assertThat(saved.getPrivateKey()).isEqualTo("priv-key-456");
    }

    @Test
    void findAllReturnsAllRootKeys() {
        RootKeyEntity key = new RootKeyEntity();
        key.setPublicKey("pub");
        key.setPrivateKey("priv");

        rootKeyRepository.save(key);

        List<RootKeyEntity> keys = rootKeyRepository.findAll();
        assertThat(keys).hasSize(1);
        assertThat(keys.get(0).getPublicKey()).isEqualTo("pub");
    }
}
