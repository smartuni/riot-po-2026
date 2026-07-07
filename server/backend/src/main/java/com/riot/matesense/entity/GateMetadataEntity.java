package com.riot.matesense.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

/**
 * An Entity for the GateMetadata
 * Represents an arbitrary key-value metadata entry associated with a Gate.
 * This allows for extensible metadata storage instead of dedicated columns per attribute.
 */
@Getter
@Setter
@Table(name = "gate_metadata")
@Entity
public class GateMetadataEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    private Long gateId;
    @Column(name = "`key`")
    private String key;
    @Column(name = "`value`")
    private String value;

    public GateMetadataEntity(Long gateId, String key, String value) {
        this.gateId = gateId;
        this.key = key;
        this.value = value;
    }

    public GateMetadataEntity() {

    }
}
