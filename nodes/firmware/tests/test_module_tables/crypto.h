#pragma once

#include <stdlib.h>
#include <string.h>

#include "clist.h"
#include "unity.h"
#include "crypto_service.h"

typedef struct {
    clist_node_t list;
    uint8_t *kid;
    size_t kid_len;
    void *data;
    size_t data_len;
    void *signature;
    size_t signature_len;
} crypto_sign_call_info_t;

typedef struct {
    unsigned int calls;
    clist_node_t call_infos;
    uint8_t *signature;
    size_t signature_len;
} crypto_ctx_t;

extern crypto_service_interface_t crypto_interface;

void crypto_init_ctx(crypto_ctx_t *ctx, const uint8_t *signature, size_t signature_len);
void crypto_free_ctx(crypto_ctx_t *ctx);
int crypto_get_sign_call(crypto_ctx_t *ctx, unsigned int index,
                         crypto_sign_call_info_t **out);
