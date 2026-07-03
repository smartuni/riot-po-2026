package com.riot.matesense.service;

import com.riot.matesense.entity.NodeEntity;
import com.riot.matesense.entity.RootKeyEntity;
import com.riot.matesense.exceptions.NodeNotFoundException;
import com.riot.matesense.exceptions.RootKeyNotFoundException;
import com.riot.matesense.model.Node;
import com.riot.matesense.model.RootKey;
import com.riot.matesense.repository.NodeRepository;
import com.riot.matesense.repository.RootKeyRepository;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.sql.Timestamp;
import java.util.List;
import java.util.stream.Collectors;

@Service
public class NodeManagementService {

    private final RootKeyRepository rootKeyRepository;
    private final NodeRepository nodeRepository;

    public NodeManagementService(RootKeyRepository rootKeyRepository, NodeRepository nodeRepository) {
        this.rootKeyRepository = rootKeyRepository;
        this.nodeRepository = nodeRepository;
    }

    @Transactional
    public void saveRootKey(RootKey rootKey) {
        List<RootKeyEntity> existing = rootKeyRepository.findAll();
        RootKeyEntity entity;
        if (existing.isEmpty()) {
            entity = new RootKeyEntity();
            entity.setCreatedAt(new Timestamp(System.currentTimeMillis()));
        } else {
            entity = existing.get(0);
        }
        entity.setPublicKey(rootKey.getPublicKey());
        entity.setPrivateKey(rootKey.getPrivateKey());
        entity.setUpdatedAt(new Timestamp(System.currentTimeMillis()));
        rootKeyRepository.save(entity);
    }

    public RootKey getRootKey() {
        List<RootKeyEntity> keys = rootKeyRepository.findAll();
        if (keys.isEmpty()) {
            throw new RootKeyNotFoundException();
        }
        RootKeyEntity entity = keys.get(0);
        return new RootKey(entity.getPublicKey(), entity.getPrivateKey());
    }

    public List<Node> getAllNodes() {
        return nodeRepository.findAll().stream()
                .map(e -> new Node(e.getId(), e.getName(), e.getPublicKey(), e.getCreatedAt()))
                .collect(Collectors.toList());
    }

    public Node addNode(String name, String publicKey) {
        NodeEntity entity = new NodeEntity();
        entity.setName(name);
        entity.setPublicKey(publicKey);
        entity.setCreatedAt(new Timestamp(System.currentTimeMillis()));
        entity.setUpdatedAt(new Timestamp(System.currentTimeMillis()));
        NodeEntity saved = nodeRepository.save(entity);
        return new Node(saved.getId(), saved.getName(), saved.getPublicKey(), saved.getCreatedAt());
    }

    @Transactional
    public void deleteNode(Long nodeId) {
        if (!nodeRepository.existsById(nodeId)) {
            throw new NodeNotFoundException(nodeId);
        }
        nodeRepository.deleteById(nodeId);
    }
}
