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
import java.util.Base64;
import java.util.List;
import java.util.stream.Collectors;

@Service
public class NodeManagementService {

    private static final String ROOT_KEY_KID = "server";
    private static final int ED25519_PUBLIC_KEY_BYTES = 32;

    private final RootKeyRepository rootKeyRepository;
    private final NodeRepository nodeRepository;

    public NodeManagementService(RootKeyRepository rootKeyRepository, NodeRepository nodeRepository) {
        this.rootKeyRepository = rootKeyRepository;
        this.nodeRepository = nodeRepository;
    }

    @Transactional
    public void saveRootKey(RootKey rootKey) {
        validatePublicKey(rootKey.getPublicKey());
        RootKeyEntity entity = rootKeyRepository.findFirstByOrderByIdAsc()
                .orElseGet(() -> {
                    RootKeyEntity newEntity = new RootKeyEntity();
                    newEntity.setCreatedAt(new Timestamp(System.currentTimeMillis()));
                    return newEntity;
                });
        entity.setKid(rootKey.getKid() != null ? rootKey.getKid() : ROOT_KEY_KID);
        entity.setPublicKey(rootKey.getPublicKey());
        entity.setPrivateKey(rootKey.getPrivateKey());
        entity.setUpdatedAt(new Timestamp(System.currentTimeMillis()));
        rootKeyRepository.save(entity);
    }

    public RootKey getRootKey() {
        RootKeyEntity entity = rootKeyRepository.findFirstByOrderByIdAsc()
                .orElseThrow(RootKeyNotFoundException::new);
        return new RootKey(entity.getKid(), entity.getPublicKey(), entity.getPrivateKey());
    }

    public List<Node> getAllNodes() {
        return nodeRepository.findAll().stream()
                .map(e -> new Node(e.getId(), e.getName(), e.getPublicKey(), e.getCreatedAt()))
                .collect(Collectors.toList());
    }

    @Transactional
    public Node addNode(String name, String publicKey) {
        validatePublicKey(publicKey);
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
        NodeEntity entity = nodeRepository.findById(nodeId)
                .orElseThrow(() -> new NodeNotFoundException(nodeId));
        nodeRepository.delete(entity);
    }

    /**
     * Validates that the public key is a valid Base64-encoded string that decodes
     * to exactly 32 bytes (Ed25519 public key size).
     */
    private void validatePublicKey(String publicKey) {
        if (publicKey == null || publicKey.isBlank()) {
            throw new IllegalArgumentException("Public key must not be blank");
        }
        byte[] decoded;
        try {
            decoded = Base64.getDecoder().decode(publicKey.trim());
        } catch (IllegalArgumentException e) {
            throw new IllegalArgumentException("Public key must be valid Base64");
        }
        if (decoded.length != ED25519_PUBLIC_KEY_BYTES) {
            throw new IllegalArgumentException(
                    "Public key must be exactly " + ED25519_PUBLIC_KEY_BYTES + " bytes (raw), got " + decoded.length
            );
        }
    }
}
