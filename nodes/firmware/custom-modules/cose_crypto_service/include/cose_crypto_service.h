#pragma once

#include <stdint.h>

#include "mutex.h"
#include "cose.h"
#include "crypto_service.h"

#include "cose_crypto_service.h"

#ifndef CONFIG_COSE_SERVICE_SCRATCH_BUF_LEN
#define CONFIG_COSE_SERVICE_SCRATCH_BUF_LEN 2048
#endif

typedef struct {
    uint8_t scratch[CONFIG_COSE_SERVICE_SCRATCH_BUF_LEN];
    mutex_t lock;
    cose_sign_enc_t cose_sign_enc;
    cose_sign_dec_t cose_sign_dec;
    cose_signature_t cose_signature;
    cose_signature_dec_t cose_signature_dec;
    cose_key_t cose_key;
} cose_crypto_service_context_t;

int cose_crypto_service_init(cose_crypto_service_context_t *context);

extern crypto_service_interface_t cose_crypto_service_interface;
