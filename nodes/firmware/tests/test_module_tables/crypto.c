#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "od.h"
#include "container.h"
#include "clist.h"
#include "unity.h"
#include "crypto_service.h"

#include "crypto.h"

void crypto_init_ctx(crypto_ctx_t *ctx, const uint8_t *signature, size_t signature_len)
{
    ctx->calls = 0;
    ctx->call_infos.next = NULL;
    ctx->signature = malloc(signature_len);
    ctx->signature_len = signature_len;

    memcpy(ctx->signature, signature, signature_len);
}

static int _free_crypto_calls(clist_node_t *node, void *arg)
{
    (void)arg;

    crypto_sign_call_info_t *info = container_of(node, crypto_sign_call_info_t, list);
    free(info->kid);
    free(info->data);
    return 0;
}

void crypto_free_ctx(crypto_ctx_t *ctx)
{
    clist_foreach(&ctx->call_infos, _free_crypto_calls, NULL);
    ctx->calls = 0;
    ctx->call_infos.next = NULL;

    free(ctx->signature);
    ctx->signature = NULL;
    ctx->signature_len = 0;

    if (ctx->signature != NULL) {
        free(ctx->signature);
    }

    // Free all call info nodes
    while (ctx->call_infos.next != NULL) {
        clist_node_t *node = ctx->call_infos.next;
        clist_lpop(&ctx->call_infos);
        free(container_of(node, crypto_sign_call_info_t, list));
    }
}

int crypto_get_sign_call(crypto_ctx_t *ctx, unsigned int index,
                         crypto_sign_call_info_t **out)
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

    *out = container_of(node, crypto_sign_call_info_t, list);
    return 0;
}

int _crypto_sign(const void *context, const uint8_t *kid, size_t kid_len,
                 const void *data, size_t data_len, void *signature,
                 size_t *signature_len)
{
    TEST_ASSERT_NOT_NULL(context);
    TEST_ASSERT_NOT_NULL(kid);
    TEST_ASSERT_NOT_NULL(data);
    TEST_ASSERT_NOT_NULL(signature_len);

    // puts("\n\n---crypto_sign called ---");
    // if (signature != NULL) {
    //     printf("Provided signature buffer (%zu bytes)\n", *signature_len);
    // } else {
    //     puts("No signature buffer provided.");
    // }

    // puts("Signer ID:");
    // od_hex_dump(signer_id, sizeof(node_id_t), 0);
    // printf("Data (%zu bytes):\n", data_len);
    // od_hex_dump(data, data_len, 0);

    crypto_ctx_t *crypto_context = (crypto_ctx_t *)context;
    crypto_sign_call_info_t *info = malloc(sizeof(crypto_sign_call_info_t));
    memset(info, 0, sizeof(crypto_sign_call_info_t));

    crypto_context->calls++;

    info->kid = malloc(kid_len);
    memcpy(info->kid, kid, kid_len);
    info->kid_len = kid_len;

    info->data = malloc(data_len);
    memcpy(info->data, data, data_len);
    info->data_len = data_len;

    info->signature = signature;
    info->signature_len = *signature_len;

    clist_rpush(&crypto_context->call_infos, &info->list);

    *signature_len = crypto_context->signature_len;

    if (crypto_context->signature_len > 0 && signature != NULL) {
        memcpy(signature, crypto_context->signature, crypto_context->signature_len);
    }

    return 0;
}

int _crypto_verify(const void *context, const uint8_t *kid, size_t kid_len,
                   const void *data, size_t data_len, const void *signature,
                   size_t signature_len)
{
    (void)kid;
    (void)kid_len;
    (void)data;
    (void)data_len;
    (void)signature;
    (void)signature_len;

    TEST_ASSERT_NOT_NULL(context);
    TEST_ASSERT_NOT_NULL(kid);
    TEST_ASSERT_NOT_NULL(data);
    TEST_ASSERT_NOT_NULL(signature);

    // For testing purposes, always return success
    return 0;
}

crypto_service_interface_t crypto_interface = {
    .sign = _crypto_sign,
    .verify = _crypto_verify
};
