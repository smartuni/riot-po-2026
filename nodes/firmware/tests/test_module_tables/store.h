#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "clist.h"
#include "unity.h"
#include "tables/types.h"

typedef struct {
    clist_node_t list;
    void *key;
    size_t key_len;
    void *data;
    size_t data_len;
} store_put_call_info_t;

typedef struct {
    unsigned int calls;
    clist_node_t call_infos;
} store_ctx;

extern store_service_interface_t store_interface;

void store_init_ctx(store_ctx *ctx);
void store_free_ctx(store_ctx *ctx);
int store_get_put_call(store_ctx *ctx, size_t index, store_put_call_info_t **out);
