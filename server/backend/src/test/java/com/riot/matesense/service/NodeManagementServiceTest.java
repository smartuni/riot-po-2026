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
            when(rootKeyRepository.findAll()).thenReturn(List.of());

            service.saveRootKey(new RootKey("pub", "priv"));

            verify(rootKeyRepository).save(argThat(entity ->
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
            existing.setPublicKey("old-pub");
            existing.setPrivateKey("old-priv");

            when(rootKeyRepository.findAll()).thenReturn(List.of(existing));

            service.saveRootKey(new RootKey("new-pub", "new-priv"));

            assertThat(existing.getPublicKey()).isEqualTo("new-pub");
            assertThat(existing.getPrivateKey()).isEqualTo("new-priv");
            verify(rootKeyRepository).save(existing);
        }

        @Test
        void getRootKeyReturnsKeyWhenExists() {
            RootKeyEntity entity = new RootKeyEntity();
            entity.setId(1L);
            entity.setPublicKey("pub");
            entity.setPrivateKey("priv");

            when(rootKeyRepository.findAll()).thenReturn(List.of(entity));

            RootKey result = service.getRootKey();

            assertThat(result.getPublicKey()).isEqualTo("pub");
            assertThat(result.getPrivateKey()).isEqualTo("priv");
        }

        @Test
        void getRootKeyThrowsWhenNotSet() {
            when(rootKeyRepository.findAll()).thenReturn(List.of());

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

            when(nodeRepository.findAll()).thenReturn(List.of(entity));

            List<Node> nodes = service.getAllNodes();
            assertThat(nodes).hasSize(1);
            assertThat(nodes.get(0).getId()).isEqualTo(1L);
            assertThat(nodes.get(0).getName()).isEqualTo("test-node");
            assertThat(nodes.get(0).getPublicKey()).isEqualTo("pub-key");
        }

        @Test
        void addNodeSavesAndReturnsNode() {
            NodeEntity savedEntity = new NodeEntity();
            savedEntity.setId(1L);
            savedEntity.setName("new-node");
            savedEntity.setPublicKey("new-key");

            when(nodeRepository.save(any(NodeEntity.class))).thenReturn(savedEntity);

            Node result = service.addNode("new-node", "new-key");

            assertThat(result.getId()).isEqualTo(1L);
            assertThat(result.getName()).isEqualTo("new-node");
            assertThat(result.getPublicKey()).isEqualTo("new-key");
            verify(nodeRepository).save(argThat(e ->
                    e.getName().equals("new-node") &&
                    e.getPublicKey().equals("new-key")
            ));
        }

        @Test
        void deleteNodeDeletesExistingNode() {
            when(nodeRepository.existsById(1L)).thenReturn(true);

            service.deleteNode(1L);

            verify(nodeRepository).deleteById(1L);
        }

        @Test
        void deleteNodeThrowsWhenNotFound() {
            when(nodeRepository.existsById(99L)).thenReturn(false);

            assertThatThrownBy(() -> service.deleteNode(99L))
                    .isInstanceOf(NodeNotFoundException.class)
                    .hasMessage("Could not find node with id 99.");
        }
    }
}
