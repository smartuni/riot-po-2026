#pragma once

#include "vfs.h"
#include "vfs_default.h"
#include "vfs_util.h"

#define IDENTITY_STORAGE_PATH VFS_DEFAULT_NVM(0) "/identities/"
#define MAX_IDENTITY_SIZE 256

#define ED25519_KEY_LEN 32
#define KID_LEN 4
#define PUBID_LEN 40
#define PUBID_SIGNATURE_LEN 80

typedef struct {
    uint8_t kid[KID_LEN];
    uint8_t key[ED25519_KEY_LEN];
} identity_t;

typedef struct {
    uint8_t cbor_payload[PUBID_LEN];
    uint8_t signature[PUBID_SIGNATURE_LEN];
} signed_identity_t;

int write_private_identity(const uint8_t *kid, size_t kid_len, const uint8_t *key, size_t key_len);
int read_private_identity(uint8_t *buffer, size_t buffer_size);
int write_root_identity(const uint8_t *kid, size_t kid_len, const uint8_t *key, size_t key_len);
int read_root_identity(uint8_t *buffer, size_t buffer_size);
int write_identity(char filename[], const uint8_t *data, size_t data_length);
int read_identity(char filename[], uint8_t *buffer, size_t buffer_size);
int identity_store_setup(void);
int get_self_node_id(uint8_t *kid_buffer, size_t kid_buffer_size);
int write_public_identity(const uint8_t *signed_id, size_t signed_id_len, const uint8_t *signature, size_t signature_len);
int get_root_identity(identity_t *identity);
int get_private_identity(identity_t *identity);
int get_public_identity(identity_t *identity);
int read_public_identity(uint8_t *buffer, size_t buffer_size);
int get_self_signed_pubid(signed_identity_t *signed_identity);
