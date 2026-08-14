#pragma once

#include "vfs.h"
#include "net/loramac.h"

#define IDENTITY_STORAGE_PATH VFS_DEFAULT_NVM(0) "/identities/"
#define MAX_IDENTITY_SIZE 256
#define ED25519_KEY_LEN (32)
#define KID_LEN 4
#define PUBID_LEN 40
#define PUBID_SIGNATURE_LEN 80
#define MAX_FILENAME_LEN 10 + 8 + 1 // "sensemate-"/"sensegate-" + id + \0

#define LORAMAC_KEY_STORAGE_PATH VFS_DEFAULT_NVM(0) "/config/loramac/"

typedef struct {
    uint8_t kid[KID_LEN];
    uint8_t key[ED25519_KEY_LEN];
} identity_t;

typedef struct {
    uint8_t cbor_payload[PUBID_LEN]; // cbor_payload deserializes to identity_t
    uint8_t signature[PUBID_SIGNATURE_LEN];
} signed_identity_t;

typedef struct {
    uint8_t joineui[LORAMAC_JOINEUI_LEN];
    uint8_t deveui[LORAMAC_DEVEUI_LEN];
    uint8_t nwkkey[LORAMAC_NWKKEY_LEN];
} loramac_keys_t;

typedef struct {
    identity_t root_identity;
    identity_t private_identity;
    signed_identity_t own_signed_identity;
    loramac_keys_t loramac_keys;
} provisioning_data_t;

/**
 * @brief Initialize the identity store
 * 
 * Initializes the identity store. This includes checking the correct directory structure
 * and all necessary files are present on the external flash.
 * If this check fails, it will format the flash, create the directory structure and drop
 * the user to a shell blocking further execution of the main thread.
 * The user then will be able to supply the necessary identity information using the
 * `identity-manager.py` script.
 * 
 * @retval 0 on success
 */
int identity_store_init(void);

/**
 * @brief Get the `JoinEUI`, `DevEUI` and `AppKey` keys from the identity store
 *
 * @param loramac_keys_out Pointer to the struct to be filled with the loramac keys
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int get_loramac_keys(loramac_keys_t *loramac_keys_out);

/**
 * @brief Get the root "CA" public key and key ID from the identity store
 *
 * @param identity_out Pointer to struct to be filled with the root identity
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int get_root_identity(identity_t *identity_out);

/**
 * @brief Get the key ID of this node from the identity store
 *
 * @param kid_buffer      Pointer to the key ID buffer
 * @param kid_buffer_size Size of the buffer
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int get_own_node_id(uint8_t *kid_buffer, size_t kid_buffer_size);

/**
 * @brief Get private key and key ID of this node from the identity store
 *
 * @param identity_out Pointer to struct to be filled with the private identity
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int get_own_private_identity(identity_t *identity_out);

/**
 * @brief Get public key and key ID of this node from the identity store
 *
 * @param identity_out Pointer to struct to be filled with this node's public identity
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int get_own_public_identity(identity_t *identity_out);

/**
 * @brief Get the CBOR encoded public identity of this node and the signature over that data from the identity store
 *
 * @param signed_identity_out Pointer to struct to be filled with this node's signed public identity
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int get_own_signed_public_identity(signed_identity_t *signed_identity_out);

/**
 * @brief Add a signed public identity (of another node) to the identity store
 *
 * @param signed_identity Pointer to struct holding the signed public identity
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int add_signed_public_identity(const signed_identity_t *signed_identity);

/**
 * @brief Initialize a `vfs_DIR` struct to use with `get_public_identities_next`
 *
 * @param dirp Pointer to uninitialized `vfs_DIR` struct 
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int get_public_identities_init(vfs_DIR *dirp);

/**
 * @brief Get the next public key and key ID from the identity store
 * 
 * This function uses a `vfs_DIR` struct initialized by `get_public_identities_init` to keep a reference
 * to the directory public identities are stored in on the stack.
 * So be sure to call that function before this one.
 *
 * @param dirp Pointer to `vfs_DIR` struct initialized with `get_public_identities_init`
 * @param identity_out Pointer to struct to be filled with the next public identity in the identity store
 *
 * @retval positive value when the next public identity was successfully read
 * @retval 0 when the end has been reached
 * @retval negative value on error
 */
int get_public_identities_next(vfs_DIR *dirp, identity_t *identity_out);
