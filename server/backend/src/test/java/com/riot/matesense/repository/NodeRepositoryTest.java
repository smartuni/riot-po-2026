package com.riot.matesense.repository;

import com.riot.matesense.entity.NodeEntity;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.orm.jpa.DataJpaTest;
import org.springframework.test.context.ActiveProfiles;

import static org.assertj.core.api.Assertions.assertThat;

@DataJpaTest
@ActiveProfiles("test")
class NodeRepositoryTest {

    @Autowired
    private NodeRepository nodeRepository;

    @Test
    void savesNodeInDatabase() {
        NodeEntity node = new NodeEntity();
        node.setName("test-node");
        node.setPublicKey("abcdef1234567890");

        NodeEntity savedNode = nodeRepository.save(node);

        assertThat(savedNode.getId()).isNotNull();
        assertThat(savedNode.getName()).isEqualTo("test-node");
        assertThat(savedNode.getPublicKey()).isEqualTo("abcdef1234567890");
    }

    @Test
    void findAllReturnsSavedNodes() {
        NodeEntity node1 = new NodeEntity();
        node1.setName("node-1");
        node1.setPublicKey("key1");

        NodeEntity node2 = new NodeEntity();
        node2.setName("node-2");
        node2.setPublicKey("key2");

        nodeRepository.save(node1);
        nodeRepository.save(node2);

        assertThat(nodeRepository.findAll()).hasSize(2);
    }

    @Test
    void deleteNodeRemovesFromDatabase() {
        NodeEntity node = new NodeEntity();
        node.setName("to-delete");
        node.setPublicKey("delete-key");

        NodeEntity saved = nodeRepository.save(node);
        assertThat(nodeRepository.existsById(saved.getId())).isTrue();

        nodeRepository.deleteById(saved.getId());
        assertThat(nodeRepository.existsById(saved.getId())).isFalse();
    }
}
