package com.riot.matesense.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

@Getter
@Setter
@Table(name = "server_key")
@Entity
public class ServerKeyEntity {

    @Id
    private Long id;

    @Column(nullable = false)
    private String kid;

    @Column(name = "private_key", nullable = false)
    private byte[] privateKey;

    @Column(name = "public_key", nullable = false)
    private byte[] publicKey;

    @Column(name = "created_at")
    private Timestamp createdAt;

    @Column(name = "updated_at")
    private Timestamp updatedAt;

    public ServerKeyEntity() {
        this.id = 1L;
    }

    public ServerKeyEntity(String kid, byte[] privateKey, byte[] publicKey) {
        this.id = 1L;
        this.kid = kid;
        this.privateKey = privateKey;
        this.publicKey = publicKey;
    }
}
