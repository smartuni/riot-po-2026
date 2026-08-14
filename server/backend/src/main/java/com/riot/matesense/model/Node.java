package com.riot.matesense.model;

import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

@Getter
@Setter
public class Node {
    private Long id;
    private String name;
    private String publicKey;
    private Timestamp createdAt;

    public Node() {}

    public Node(Long id, String name, String publicKey, Timestamp createdAt) {
        this.id = id;
        this.name = name;
        this.publicKey = publicKey;
        this.createdAt = createdAt;
    }
}
