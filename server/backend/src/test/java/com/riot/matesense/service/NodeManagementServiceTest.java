package com.riot.matesense.service;

import com.riot.matesense.entity.NodeEntity;
import com.riot.matesense.entity.RootKeyEntity;
import com.riot.matesense.exceptions.NodeNotFoundException;
import com.riot.matesense.exceptions.RootKeyNotFoundException;
import com.riot.matesense.model.Node;
import com.riot.matesense.model.RootKey;
import com.riot.matesense.repository.NodeRepository;
import com.riot.matesense.repository.RootKeyRepository;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;

import java.sql.Timestamp;
import java.util.List;
import java.util.Optional;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.argThat;
import static org.mockito.Mockito.*;

@ExtendWith(MockitoExtension.class)
class NodeManagementServiceTest {

    @Mock
    private RootKeyRepository rootKeyRepository;

    @Mock
    private NodeRepository nodeRepository;

    private NodeManagementService service;

    @BeforeEach
    void setUp() {
        service = new NodeManagementService(rootKeyRepository, nodeRepository);
    }

    @Nested
    class RootKeyTests {

        @Test
        void saveRootKeyCreatesNewEntityWhenNoneExists() {
            when(rootKeyRepository.findFirstByOrderByIdAsc()).thenReturn(Optional.empty());

            service.saveRootKey(new RootKey("server", "pub", "priv"));

            verify(rootKeyRepository).save(argThat(entity ->
                    entity.getKid().equals("server") &&
                    entity.getPublicKey().equals("pub") &&
                    entity.getPrivateKey().equals("priv") &&
                    entity.getCreatedAt() != null &&
                    entity.getUpdatedAt() != null
            ));
        }

        @Test
        void saveRootKeyUpdatesExistingEntity() {
            RootKeyEntity existing = new RootKeyEntity();
            existing.setId(1L);
            existing.setKid("server");
            existing.setPublicKey("old-pub");
            existing.setPrivateKey("old-priv");

            when(rootKeyRepository.findFirstByOrderByIdAsc()).thenReturn(Optional.of(existing));

            service.saveRootKey(new RootKey("server", "new-pub", "new-priv"));

            assertThat(existing.getPublicKey()).isEqualTo("new-pub");
            assertThat(existing.getPrivateKey()).isEqualTo("new-priv");
            verify(rootKeyRepository).save(existing);
        }

        @Test
        void saveRootKeyDefaultsKidToServerWhenNull() {
            when(rootKeyRepository.findFirstByOrderByIdAsc()).thenReturn(Optional.empty());

            service.saveRootKey(new RootKey(null, "pub", "priv"));

            verify(rootKeyRepository).save(argThat(entity ->
                    entity.getKid().equals("server")
            ));
        }

        @Test
        void getRootKeyReturnsKeyWhenExists() {
            RootKeyEntity entity = new RootKeyEntity();
            entity.setId(1L);
            entity.setKid("server");
            entity.setPublicKey("pub");
            entity.setPrivateKey("priv");

            when(rootKeyRepository.findFirstByOrderByIdAsc()).thenReturn(Optional.of(entity));

            RootKey result = service.getRootKey();

            assertThat(result.getKid()).isEqualTo("server");
            assertThat(result.getPublicKey()).isEqualTo("pub");
            assertThat(result.getPrivateKey()).isEqualTo("priv");
        }

        @Test
        void getRootKeyThrowsWhenNotSet() {
            when(rootKeyRepository.findFirstByOrderByIdAsc()).thenReturn(Optional.empty());

            assertThatThrownBy(() -> service.getRootKey())
                    .isInstanceOf(RootKeyNotFoundException.class)
                    .hasMessage("Root key has not been set up yet.");
        }
    }

    @Nested
    class NodeTests {

        @Test
        void getAllNodesReturnsEmptyListWhenNone() {
            when(nodeRepository.findAll()).thenReturn(List.of());

            assertThat(service.getAllNodes()).isEmpty();
        }

        @Test
        void getAllNodesReturnsAllNodes() {
            NodeEntity entity = new NodeEntity();
            entity.setId(1L);
            entity.setName("test-node");
            entity.setPublicKey("pub-key");
            entity.setCreatedAt(Timestamp.valueOf("2026-01-01 00:00:00"));

            when(nodeRepository.findAll()).thenReturn(List.of(entity));

            List<Node> nodes = service.getAllNodes();
            assertThat(nodes).hasSize(1);
            assertThat(nodes.get(0).getId()).isEqualTo(1L);
            assertThat(nodes.get(0).getName()).isEqualTo("test-node");
            assertThat(nodes.get(0).getPublicKey()).isEqualTo("pub-key");
            assertThat(nodes.get(0).getCreatedAt()).isEqualTo(Timestamp.valueOf("2026-01-01 00:00:00"));
        }

        @Test
        void addNodeSavesAndReturnsNode() {
            // 32-byte Ed25519 public key encoded as Base64
            String validKey = java.util.Base64.getEncoder().encodeToString(new byte[32]);

            NodeEntity savedEntity = new NodeEntity();
            savedEntity.setId(1L);
            savedEntity.setName("new-node");
            savedEntity.setPublicKey(validKey);
            savedEntity.setCreatedAt(Timestamp.valueOf("2026-01-01 12:00:00"));

            when(nodeRepository.save(any(NodeEntity.class))).thenReturn(savedEntity);

            Node result = service.addNode("new-node", validKey);

            assertThat(result.getId()).isEqualTo(1L);
            assertThat(result.getName()).isEqualTo("new-node");
            assertThat(result.getPublicKey()).isEqualTo(validKey);
            assertThat(result.getCreatedAt()).isEqualTo(Timestamp.valueOf("2026-01-01 12:00:00"));
            verify(nodeRepository).save(argThat(e ->
                    e.getName().equals("new-node") &&
                    e.getPublicKey().equals(validKey)
            ));
        }

        @Test
        void addNodeRejectsInvalidBase64Key() {
            assertThatThrownBy(() -> service.addNode("test", "not-valid-base64!!!"))
                    .isInstanceOf(IllegalArgumentException.class)
                    .hasMessageContaining("Public key must be valid Base64");
        }

        @Test
        void addNodeRejectsWrongKeySize() {
            // 16-byte key — too short for Ed25519
            String shortKey = java.util.Base64.getEncoder().encodeToString(new byte[16]);

            assertThatThrownBy(() -> service.addNode("test", shortKey))
                    .isInstanceOf(IllegalArgumentException.class)
                    .hasMessageContaining("Public key must be exactly 32 bytes");
        }

        @Test
        void addNodeRejectsBlankKey() {
            assertThatThrownBy(() -> service.addNode("test", ""))
                    .isInstanceOf(IllegalArgumentException.class)
                    .hasMessageContaining("Public key must not be blank");
        }

        @Test
        void deleteNodeDeletesExistingNode() {
            NodeEntity entity = new NodeEntity();
            entity.setId(1L);
            when(nodeRepository.findById(1L)).thenReturn(Optional.of(entity));

            service.deleteNode(1L);

            verify(nodeRepository).delete(entity);
        }

        @Test
        void deleteNodeThrowsWhenNotFound() {
            when(nodeRepository.findById(99L)).thenReturn(Optional.empty());

            assertThatThrownBy(() -> service.deleteNode(99L))
                    .isInstanceOf(NodeNotFoundException.class)
                    .hasMessage("Could not find node with id 99.");
        }
    }
}
