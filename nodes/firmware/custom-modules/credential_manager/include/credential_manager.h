#pragma once

#include "store_service.h"

/**
 * @brief Type of keys in the credential manager
 */
typedef enum {
    /**< Public part of key */
    CREDENTIAL_PUBLIC   = 0,

    /**< Private part of a key */
    CREDENTIAL_PRIVATE  = 1
} credential_manager_type_t;

/**
 * @brief Initialize the credential manager.
 *
 * @param store_service Pointer to the store service to use to store credentials
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int credential_manager_init(store_service_t *store_service);

/**
 * @brief Get a key from the credential manager
 *
 * @param kid       Pointer to the Key ID to get
 * @param kid_len   Length of @p kid
 * @param type      Type of key to get
 * @param key       Buffer to store the key. If NULL, only the size will be informed in
 *                  @p key_len
 * @param key_len   Pointer to place the size of the key
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int credential_manager_get_key(const uint8_t *kid, size_t kid_len,
                               credential_manager_type_t type, uint8_t *key,
                               size_t *key_len);
/**
 * @brief Add a key to the credential manager.
 *
 * This stores a key (public or private) identified by a Key ID (KID) into the
 * underlying store service previously provided to the credential manager via
 * @ref credential_manager_init.
 *
 * @param kid       Pointer to the Key ID to store
 * @param kid_len   Length of @p kid
 * @param type      Type of key to store (public or private)
 * @param key       Pointer to the key bytes to store
 * @param key_len   Length of @p key in bytes
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int credential_manager_add_key(const uint8_t *kid, size_t kid_len,
                               credential_manager_type_t type, const uint8_t *key,
                               size_t key_len);

/**
 * @brief Delete a key from the credential manager.
 *
 * Remove a stored key (public or private) identified by the given Key ID from the
 * underlying store. If the key does not exist, implementations may return an error or
 * treat it as a no-op depending on their policy.
 *
 * @param kid       Pointer to the Key ID to delete
 * @param kid_len   Length of @p kid
 * @param type      Type of key to delete (public or private)
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int credential_manager_delete_key(const uint8_t *kid, size_t kid_len,
                                  credential_manager_type_t type);
