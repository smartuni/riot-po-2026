#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "od.h"
#include "container.h"
#include "clist.h"
#include "unity.h"
#include "store.h"
#include "store_service.h"

typedef struct {
    clist_node_t *node;
    store_service_query_t query;
} store_iterator_t;

void store_init_ctx(store_ctx *ctx)
{
    ctx->calls = 0;
    ctx->call_infos.next = NULL;
}

static int _free_store_calls(clist_node_t *node, void *arg)
{
    (void)arg;

    store_put_call_info_t *info = container_of(node, store_put_call_info_t, list);
    free(info->key);
    free(info->data);
    return 0;
}

void store_free_ctx(store_ctx *ctx)
{
    clist_foreach(&ctx->call_infos, _free_store_calls, NULL);
    ctx->calls = 0;

    // Free all call info nodes
    while (ctx->call_infos.next != NULL) {
        clist_node_t *node = clist_lpop(&ctx->call_infos);
        free(container_of(node, store_put_call_info_t, list));
    }
}

int store_get_put_call(store_ctx *ctx, size_t index, store_put_call_info_t **out)
{
    if (index >= ctx->calls || ctx->call_infos.next == NULL) {
        return -1;
    }

    clist_node_t *node = ctx->call_infos.next->next;
    for (size_t i = 0; i < index; i++) {
        node = node->next;

        if (node == NULL) {
            return -1;
        }
    }

    *out = container_of(node, store_put_call_info_t, list);
    return 0;
}

int _store_put(const void *context, const uint8_t *key, size_t key_len, const void * data,
              size_t data_len)
{
    (void)key_len;
    (void)data_len;

    TEST_ASSERT_NOT_NULL(context);
    TEST_ASSERT_NOT_NULL(key);
    TEST_ASSERT_NOT_NULL(data);

    store_ctx *store_context = (store_ctx *)context;
    store_put_call_info_t *info = (store_put_call_info_t *)malloc(sizeof(store_put_call_info_t));
    memset(info, 0, sizeof(store_put_call_info_t));

    store_context->calls++;

    info->key = malloc(key_len);
    memcpy(info->key, key, key_len);
    info->key_len = key_len;

    info->data = malloc(data_len);
    memcpy(info->data, data, data_len);
    info->data_len = data_len;

    clist_rpush(&store_context->call_infos, &info->list);

    // puts("\n\n---store_put called ---");
    // printf("Key (%zu bytes):\n", key_len);
    // od_hex_dump(key, key_len, 0);
    // printf("Data (%zu bytes):\n", data_len);
    // od_hex_dump(data, data_len, 0);

    return 0;
}

bool _call_matches_key(store_put_call_info_t *call, const uint8_t *key, size_t key_len)
{
    if (call->key_len != key_len) {
        return false;
    }

    return memcmp(call->key, key, key_len) == 0;
}

int _store_get(const void *context, const uint8_t *key, size_t key_len, void * data,
              size_t data_len)
{
    TEST_ASSERT_NOT_NULL(context);
    TEST_ASSERT_NOT_NULL(key);
    TEST_ASSERT_NOT_NULL(data);

    store_ctx *ctx = (store_ctx *) context;

    clist_node_t *node = ctx->call_infos.next;
    if (!node) {
        return -1;
    }

    do {
        node = node->next;
        store_put_call_info_t *call = container_of(node, store_put_call_info_t, list);
        if (!_call_matches_key(call, key, key_len)) {
            continue;
        }

        if (data_len < call->data_len) {
            return -1;
        }

        memcpy(data, call->data, call->data_len);
        return 0;
    } while (node != ctx->call_infos.next);

    return -1;
}

int _store_iterator_init(const void *context, store_service_iterator_t *iterator,
                         const store_service_query_t *query)
{
    TEST_ASSERT_NOT_NULL(context);
    TEST_ASSERT_NOT_NULL(iterator);
    TEST_ASSERT_NOT_NULL(query);

    store_ctx *ctx = (store_ctx *) context;
    store_iterator_t *iter = (store_iterator_t *) iterator;
    iter->query = *query;
    iter->node = ctx->call_infos.next;

    return 0;
}

size_t _store_iterator_size(const void *context)
{
    TEST_ASSERT_NOT_NULL(context);
    return sizeof(store_iterator_t);
}

int _store_iterator_next(const void *context, store_service_iterator_t *iterator,
                         uint8_t *key, size_t *key_len, void *data, size_t *data_len)
{
    TEST_ASSERT_NOT_NULL(context);
    TEST_ASSERT_NOT_NULL(iterator);
    TEST_ASSERT_NOT_NULL(key);
    TEST_ASSERT_NOT_NULL(key_len);
    TEST_ASSERT_NOT_NULL(data);
    TEST_ASSERT_NOT_NULL(data_len);

    store_ctx *ctx = (store_ctx *) context;
    store_iterator_t *iter = (store_iterator_t *) iterator;

    clist_node_t *node = iter->node;
    do {
        node = node->next;
        store_put_call_info_t *call = container_of(node, store_put_call_info_t, list);
        if (!store_service_key_matches_query(call->key, call->key_len, &iter->query)) {
            continue;
        }

        iter->node = node;

        if (*key_len < call->key_len || *data_len < call->data_len) {
            return -1;
        }

        memcpy(key, call->key, call->key_len);
        *key_len = call->key_len;

        memcpy(data, call->data, call->data_len);
        *data_len = call->data_len;

        return 0;

    } while (node != ctx->call_infos.next);

    return -1;
}

store_service_interface_t store_interface = {
    .get = _store_get,
    .put = _store_put,
    .iterator_init = _store_iterator_init,
    .iterator_size = _store_iterator_size,
    .iterator_next = _store_iterator_next
};
