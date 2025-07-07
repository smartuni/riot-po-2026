//
// Created by Malte Blöing on 02.06.25.
//

#ifndef ED25519_KEY_ENTRY_H
#define ED25519_KEY_ENTRY_H

#define MAX_KID_LEN 32

typedef struct {
    uint8_t kid[MAX_KID_LEN];     // z. B. "device-01"
    size_t kid_len;

    uint8_t public_key[32];       // Ed25519 public key
} ed25519_public_key_entry_t;

#endif //ED25519_KEY_ENTRY_H
