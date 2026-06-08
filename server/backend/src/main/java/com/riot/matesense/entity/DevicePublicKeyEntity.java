package com.riot.matesense.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

@Getter
@Setter
@Table(name = "device_public_keys")
@Entity
public class DevicePublicKeyEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false, unique = true)
    private String kid;

    @Column(name = "public_key", nullable = false)
    private byte[] publicKey;

    @Column(name = "created_at")
    private Timestamp createdAt;

    @Column(name = "updated_at")
    private Timestamp updatedAt;

    public DevicePublicKeyEntity() {
    }

    public DevicePublicKeyEntity(String kid, byte[] publicKey) {
        this.kid = kid;
        this.publicKey = publicKey;
    }
}
