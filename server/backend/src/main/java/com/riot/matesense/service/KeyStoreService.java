package com.riot.matesense.service;

import com.riot.matesense.entity.DevicePublicKeyEntity;
import com.riot.matesense.entity.ServerKeyEntity;
import com.riot.matesense.repository.DevicePublicKeyRepository;
import com.riot.matesense.repository.ServerKeyRepository;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.sql.Timestamp;
import java.time.Instant;
import java.util.List;
import java.util.Optional;

@Service
public class KeyStoreService {

    private final DevicePublicKeyRepository devicePublicKeyRepository;
    private final ServerKeyRepository serverKeyRepository;

    public KeyStoreService(DevicePublicKeyRepository devicePublicKeyRepository,
                           ServerKeyRepository serverKeyRepository) {
        this.devicePublicKeyRepository = devicePublicKeyRepository;
        this.serverKeyRepository = serverKeyRepository;
    }

    @Transactional(readOnly = true)
    public Optional<byte[]> getPublicKey(String kid) {
        return devicePublicKeyRepository.findByKid(kid)
                .map(DevicePublicKeyEntity::getPublicKey);
    }

    @Transactional
    public void addPublicKey(String kid, byte[] publicKey) {
        if (publicKey == null || publicKey.length != 32) {
            throw new IllegalArgumentException("Ed25519 public key must be exactly 32 bytes");
        }
        DevicePublicKeyEntity entity = devicePublicKeyRepository.findByKid(kid)
                .orElse(new DevicePublicKeyEntity(kid, publicKey));
        entity.setPublicKey(publicKey);
        entity.setUpdatedAt(Timestamp.from(Instant.now()));
        if (entity.getCreatedAt() == null) {
            entity.setCreatedAt(Timestamp.from(Instant.now()));
        }
        devicePublicKeyRepository.save(entity);
    }

    @Transactional
    public void deletePublicKey(String kid) {
        devicePublicKeyRepository.deleteByKid(kid);
    }

    @Transactional(readOnly = true)
    public List<DevicePublicKeyEntity> getAllPublicKeys() {
        return devicePublicKeyRepository.findAll();
    }

    @Transactional(readOnly = true)
    public boolean hasPublicKey(String kid) {
        return devicePublicKeyRepository.existsByKid(kid);
    }

    @Transactional(readOnly = true)
    public Optional<ServerKeyEntity> getServerKey() {
        return serverKeyRepository.findById(1L);
    }

    @Transactional(readOnly = true)
    public boolean hasServerKey() {
        return serverKeyRepository.existsById(1L);
    }

    @Transactional
    public void setServerKey(String kid, byte[] privateKey, byte[] publicKey) {
        if (privateKey == null || privateKey.length != 64) {
            throw new IllegalArgumentException("Ed25519 private key seed must be exactly 64 bytes");
        }
        if (publicKey == null || publicKey.length != 32) {
            throw new IllegalArgumentException("Ed25519 public key must be exactly 32 bytes");
        }
        ServerKeyEntity entity = serverKeyRepository.findById(1L)
                .orElse(new ServerKeyEntity());
        entity.setKid(kid);
        entity.setPrivateKey(privateKey);
        entity.setPublicKey(publicKey);
        entity.setUpdatedAt(Timestamp.from(Instant.now()));
        if (entity.getCreatedAt() == null) {
            entity.setCreatedAt(Timestamp.from(Instant.now()));
        }
        serverKeyRepository.save(entity);
    }
}
