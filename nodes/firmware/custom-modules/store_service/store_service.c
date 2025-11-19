#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "store_service.h"

int store_service_get(store_service_t *service, const uint8_t *key, size_t key_len,
                      void *data, size_t data_len)
{
    assert(service != NULL);
    assert(key != NULL);
    assert(data != NULL);

    return service->interface.get(service->context, key, key_len, data, data_len);
}

int store_service_put(store_service_t *service, const uint8_t *key, size_t key_len,
                      const void *data, size_t data_len)
{
    assert(service != NULL);
    assert(key != NULL);
    assert(data != NULL);

    return service->interface.put(service->context, key, key_len, data, data_len);
}

int store_service_delete(store_service_t *service, const uint8_t *key, size_t key_len)
{
    assert(service != NULL);
    assert(key != NULL);

    return service->interface.delete(service->context, key, key_len);
}

int store_service_iterator_init(store_service_t *service,
                                void *iterator,
                                store_service_query_t *query)
{
    assert(service != NULL);
    assert(iterator != NULL);
    assert(query != NULL);

    return service->interface.iterator_init(service->context, iterator, query);
}

int store_service_iterator_next(store_service_t *service,
                                void *iterator, uint8_t *key,
                                size_t *key_len, uint8_t *data, size_t *data_len)
{
    assert(service != NULL);
    assert(iterator != NULL);
    assert(key != NULL);
    assert(key_len != NULL);
    assert(data_len != NULL);

    return service->interface.iterator_next(service->context, iterator, key, key_len,
                                            data, data_len);
}

bool store_service_key_matches_query(const uint8_t *key, size_t key_len,
                                     const store_service_query_t *query)
{
    assert(key != NULL);
    assert(query != NULL);

    if (query->key_len != key_len) {
        return false;
    }

    const uint8_t *query_key = query->key;
    const uint8_t *query_mask = query->key_mask;

    if (query_mask == NULL) {
        // We need an exact match
        return memcmp(key, query_key, key_len) == 0;
    }

    // We need to mask the match
    for (size_t i = 0; i < key_len; i++) {
        if (((key[i] ^ query_key[i]) & query_mask[i]) != 0) {
            return false;
        }
    }

    return true;

}
