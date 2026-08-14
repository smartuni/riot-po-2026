package com.riot.matesense.model;

import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class RootKey {
    private String kid;
    private String publicKey;
    private String privateKey;

    public RootKey() {}

    public RootKey(String kid, String publicKey, String privateKey) {
        this.kid = kid;
        this.publicKey = publicKey;
        this.privateKey = privateKey;
    }
}
