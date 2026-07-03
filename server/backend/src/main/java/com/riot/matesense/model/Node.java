package com.riot.matesense.model;

import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class Node {
    private Long id;
    private String name;
    private String publicKey;

    public Node() {}

    public Node(Long id, String name, String publicKey) {
        this.id = id;
        this.name = name;
        this.publicKey = publicKey;
    }
}
