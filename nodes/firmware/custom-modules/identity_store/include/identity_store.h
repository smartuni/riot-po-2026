#pragma once

#include "vfs.h"

#define IDENTITY_STORAGE_PATH VFS_DEFAULT_NVM(0) "/identities/"
#define MAX_IDENTITY_SIZE 256
#define ED25519_KEY_LEN (32)
#define KID_LEN 4
#define PUBID_LEN 40
#define PUBID_SIGNATURE_LEN 80
#define MAX_FILENAME_LEN 10 + 8 + 1 // "sensemate-"/"sensegate-" + id + \0

typedef struct {
    uint8_t kid[KID_LEN];
    uint8_t key[ED25519_KEY_LEN];
} identity_t;

typedef struct {
    uint8_t cbor_payload[PUBID_LEN]; // cbor_payload deserializes to identity_t
    uint8_t signature[PUBID_SIGNATURE_LEN];
} signed_identity_t;

typedef struct {
    identity_t root_identity;
    identity_t private_identity;
    signed_identity_t own_signed_identity;
} provisioning_data_t;

int identity_store_init(void);
int get_root_identity(identity_t *identity_out);
int get_own_node_id(uint8_t *kid_buffer, size_t kid_buffer_size);
int get_own_private_identity(identity_t *identity_out);
int get_own_public_identity(identity_t *identity_out);
int get_own_signed_public_identity(signed_identity_t *signed_identity_out);
int add_signed_public_identity(const signed_identity_t *signed_identity);
int get_public_identities_init(vfs_DIR *dirp);
int get_public_identities_next(vfs_DIR *dirp, identity_t *identity_out);
