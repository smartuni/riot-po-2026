#include <assert.h>
#include <stddef.h>

#include "mutex.h"
#include "crypto_service.h"
#include "cose.h"
#include "credential_manager.h"

#include "cose_crypto_service.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#define ED25519_KEY_LEN (32)

int cose_crypto_service_init(cose_crypto_service_context_t *context)
{
    assert(context != NULL);
    mutex_init(&context->lock);

    return 0;
}

static int _sign_payload(cose_crypto_service_context_t *ctx, const uint8_t *kid,
                         size_t kid_len, const uint8_t *payload, size_t payload_len,
                         uint8_t **signature, size_t *signature_len)
{
    uint8_t private_key[ED25519_KEY_LEN];
    size_t private_key_len = ED25519_KEY_LEN;
    int result = credential_manager_get_key(kid, kid_len, CREDENTIAL_PRIVATE,
                                            private_key, &private_key_len);

    if (result != 0) {
        DEBUG("_sign_payload: could not get private key\n");
        return -1;
    }

    uint8_t public_key[ED25519_KEY_LEN];
    size_t public_key_len = ED25519_KEY_LEN;
    result = credential_manager_get_key(kid, kid_len, CREDENTIAL_PUBLIC, public_key,
                                        &public_key_len);
    if (result != 0) {
        DEBUG("_sign_payload: could not get public key\n");
        return -1;
    }

    // Initialize the key
    cose_key_init(&ctx->cose_key);
    cose_key_set_keys(&ctx->cose_key, COSE_EC_CURVE_ED25519, COSE_ALGO_EDDSA, public_key,
                      NULL, private_key);

    // kid is not actually modified, just an API definition issue
    cose_key_set_kid(&ctx->cose_key, (uint8_t *)kid, kid_len);

    // we send the payload externally
    cose_sign_init(&ctx->cose_sign_enc, COSE_FLAGS_UNTAGGED | COSE_FLAGS_EXTDATA);
    cose_signature_init(&ctx->cose_signature);

    // set the payload we want to sign and add a single signer (Sign1 message)
    cose_sign_set_payload(&ctx->cose_sign_enc, payload, payload_len);
    cose_sign_add_signer(&ctx->cose_sign_enc, &ctx->cose_signature, &ctx->cose_key);

    uint8_t *_signature;
    ssize_t _signature_len = cose_sign_encode(&ctx->cose_sign_enc, ctx->scratch,
                                              sizeof(ctx->scratch), &_signature);
    if (_signature_len <= 0) {
        DEBUG("_sign_payload: failed to encode COSE signature\n");
        return -1;
    }

    *signature_len = (size_t)_signature_len;
    *signature = _signature;
    return 0;
}

static int _cose_crypto_service_sign(const void *context, const uint8_t *kid,
                                     size_t kid_len, const void *data, size_t data_len,
                                     void *signature, size_t *signature_len)
{
    assert(context != NULL);
    assert(kid != NULL);
    assert(data != NULL);
    assert(signature_len != NULL);

    int result;
    cose_crypto_service_context_t *ctx = (cose_crypto_service_context_t *)context;
    uint8_t *_signature;
    size_t _signature_len;

    DEBUG("_cose_crypto_service_sign: signing payload with COSE\n");

    mutex_lock(&ctx->lock);

    // attempt to create signature in the scratch buffer
    result = _sign_payload(ctx, kid, kid_len, data, data_len, &_signature,
                           &_signature_len);
    if (result != 0) {
        DEBUG("_cose_crypto_service_sign: failed to create the signature\n");
        result = -1;
        goto unlock_and_return;
    }

    // if this was a call to estimate the buffer size, update length and leave
    if (signature == NULL) {
        DEBUG("_cose_crypto_service_sign: signature created but not copied\n");
        *signature_len = _signature_len;
        result = 0;
        goto unlock_and_return;
    }

    // we need to copy the signature, check if we have space
    if (*signature_len < _signature_len) {
        DEBUG("_cose_crypto_service_sign: there's not enough space for signature\n");
        result = -1;
        goto unlock_and_return;
    }

    // copy the signature
    DEBUG("_cose_crypto_service_sign: signature creted and copied\n");
    memcpy((uint8_t *)signature, _signature, _signature_len);
    *signature_len = _signature_len;
    result = 0;

unlock_and_return:
    mutex_unlock(&ctx->lock);
    return result;
}

static int _cose_crypto_service_verify(const void *context, const uint8_t *kid,
                                       size_t kid_len, const void *data, size_t data_len,
                                       const void *signature, size_t signature_len)
{
    assert(context != NULL);
    assert(kid != NULL);
    assert(data != NULL);
    assert(signature != NULL);

    int result;
    cose_crypto_service_context_t *ctx = (cose_crypto_service_context_t *)context;

    DEBUG("_cose_crypto_service_verify: verifying COSE signature\n");

    mutex_lock(&ctx->lock);

    result = cose_sign_decode(&ctx->cose_sign_dec, signature, signature_len);
    if (result != 0) {
        DEBUG("_cose_crypto_service_verify: failed to decode signature\n");
        result = -1;
        goto unlock_and_return;
    }

    cose_sign_signature_iter_init(&ctx->cose_signature_dec);
    bool filled = cose_sign_signature_iter(&ctx->cose_sign_dec, &ctx->cose_signature_dec);
    if (!filled) {
        DEBUG("_cose_crypto_service_verify: no signatures found in COSE structure\n");
        result = -1;
        goto unlock_and_return;
    }

    // get the key ID from the signature
    const uint8_t *kid_in_signature;
    ssize_t kid_in_signature_len = cose_signature_decode_kid(&ctx->cose_signature_dec,
                                                             &kid_in_signature);
    if (kid_in_signature_len <= 0) {
        DEBUG("_cose_crypto_service_verify: no key ID found in the signature\n");
        result = -1;
        goto unlock_and_return;
    }

    // check that the key ID matches the expected key ID
    if ((size_t)kid_in_signature_len != kid_len ||
        memcmp(kid, kid_in_signature, kid_len) != 0) {
        DEBUG("_cose_crypto_service_verify: key ID does not match the expected ID\n");
        result = -1;
        goto unlock_and_return;
    }

    // initialize the key to verify the signature using the key ID we just got
    cose_key_init(&ctx->cose_key);
    uint8_t public_key[ED25519_KEY_LEN];
    size_t public_key_len = ED25519_KEY_LEN;
    result = credential_manager_get_key(kid_in_signature, kid_in_signature_len,
                                        CREDENTIAL_PUBLIC, public_key, &public_key_len);
    if (result != 0) {
        DEBUG("_cose_crypto_service_verify: public key not found\n");
        result = -1;
        goto unlock_and_return;
    }

    cose_key_set_keys(&ctx->cose_key, COSE_EC_CURVE_ED25519, COSE_ALGO_EDDSA,
                      (uint8_t *)public_key, NULL, NULL);

    // the payload is external, so we need to set it for the verification
    cose_sign_decode_set_payload(&ctx->cose_sign_dec, data, data_len);

    result = cose_sign_verify(&ctx->cose_sign_dec, &ctx->cose_signature_dec, &ctx->cose_key,
                              ctx->scratch, sizeof(ctx->scratch));
    if (result != 0) {
        DEBUG("_cose_crypto_service_verify: verification of signature failed\n");
        result = -1;
        goto unlock_and_return;
    }

    DEBUG("_cose_crypto_service_verify: verification successful\n");
    result = 0;

unlock_and_return:
    mutex_unlock(&ctx->lock);
    return result;
}

crypto_service_interface_t cose_crypto_service_interface = {
    .sign = _cose_crypto_service_sign,
    .verify = _cose_crypto_service_verify
};
