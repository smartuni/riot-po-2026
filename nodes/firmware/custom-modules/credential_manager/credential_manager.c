#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "include/credential_manager.h"
#include "mutex.h"
#include "store_service.h"

#include "credential_manager.h"

static store_service_t _store;
static mutex_t _lock;

// For now single key
#define ED25519_KEY_LEN (32)

static void _get_store_key(const uint8_t *kid, size_t kid_len,
                           credential_manager_type_t type, uint8_t *store_key,
                           size_t store_key_len)
{
    assert(kid != NULL);
    assert(store_key != NULL);
    assert(store_key_len >= kid_len + 1);

    store_key[0] = type;
    memcpy(&store_key[1], kid, kid_len);
}

int credential_manager_init(store_service_t *store_service)
{
    assert(store_service);
    _store = *store_service;
    mutex_init(&_lock);
    return 0;
}

int credential_manager_get_key(const uint8_t *kid, size_t kid_len,
                               credential_manager_type_t type, uint8_t *key,
                               size_t *key_len)
{
    assert(kid != NULL);
    assert(key_len != NULL);

    int result = 0;
    size_t store_key_len = kid_len + 1;
    uint8_t store_key[store_key_len];

    mutex_lock(&_lock);
    _get_store_key(kid, kid_len, type, store_key, store_key_len);
    size_t data_len = ED25519_KEY_LEN;

    if (key == NULL) {
        *key_len = data_len;
        result = 0;
        goto unlock_and_return;
    }

    if (*key_len < data_len) {
        *key_len = data_len;
        result = -1;
        goto unlock_and_return;
    }

    result = store_service_get(&_store, store_key, store_key_len, key, data_len);
    if (result != 0) {
        result = -1;
        goto unlock_and_return;
    }

    *key_len = data_len;

unlock_and_return:
    mutex_unlock(&_lock);
    return result;
}

int credential_manager_add_key(const uint8_t *kid, size_t kid_len,
                               credential_manager_type_t type, const uint8_t *key,
                               size_t key_len)
{
    assert(kid != NULL);
    assert(key != NULL);

    int result = 0;
    size_t store_key_len = kid_len + 1;
    uint8_t store_key[store_key_len];

    mutex_lock(&_lock);

    _get_store_key(kid, kid_len, type, store_key, store_key_len);
    result = store_service_put(&_store, store_key, store_key_len, key, key_len);
    if (result != 0) {
        result = -1;
        goto unlock_and_return;
    }

    result = 0;

unlock_and_return:
    mutex_unlock(&_lock);
    return result;
}

int credential_manager_delete_key(const uint8_t *kid, size_t kid_len,
                                  credential_manager_type_t type)
{
    assert(kid != NULL);

    int result = 0;
    size_t store_key_len = kid_len + 1;
    uint8_t store_key[store_key_len];

    mutex_lock(&_lock);

    _get_store_key(kid, kid_len, type, store_key, store_key_len);
    result = store_service_delete(&_store, store_key, store_key_len);
    if (result != 0) {
        result = -1;
        goto unlock_and_return;
    }

    result = 0;

unlock_and_return:
    mutex_unlock(&_lock);
    return result;
}
