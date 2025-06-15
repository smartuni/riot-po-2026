//
// Created by Malte Blöing on 02.06.25.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cose.h"
#include "cose/sign.h"
#include "cose/crypto.h"
#include "cbor.h"
#include "cborjson.h"
#include "random.h"

#include "cose-service.h"
#include "public_keys.h"
#include "private_key.h"


/* COSE structs */
static cose_sign_enc_t sign;
static cose_sign_dec_t verify;
static cose_signature_t signature1;
static cose_key_t signer1;
static cose_key_t verifyer;

/* Buffer */
static uint8_t sign_buf[2048];
static uint8_t v_buf[2048];

/* COSE Header KID */
#define COSE_HEADER_KID 4

int sign_payload(const uint8_t *payload, size_t payload_len,
                 uint8_t **out_buf, size_t *out_len)
{
    if (!payload || payload_len == 0 || !out_buf || !out_len)
    {
        return COSE_SIGN_ERR_INVALID_ARGS;
    }

    cose_key_init(&signer1);
    cose_key_set_keys(&signer1, COSE_EC_CURVE_ED25519, COSE_ALGO_EDDSA,
                      ed25519_public_key, NULL, ed25519_secret_key);
    cose_key_set_kid(&signer1, (uint8_t *)ownId, sizeof(ownId) - 1);

    cose_sign_init(&sign, COSE_FLAGS_UNTAGGED);
    cose_signature_init(&signature1);

    // Signiere jetzt beliebige Binärdaten
    cose_sign_set_payload(&sign, payload, payload_len);
    cose_sign_add_signer(&sign, &signature1, &signer1);

    ssize_t len = cose_sign_encode(&sign, sign_buf, sizeof(sign_buf), out_buf);
    if (len < 0)
    {
        printf("Signing faild with: %i",len);
        return COSE_SIGN_ERR_ENCODE_FAILED;
    }

    *out_len = len;
    return COSE_SIGN_SUCCESS;
}

const uint8_t* get_public_key_for_kid(const uint8_t *kid, size_t kid_len)
{
    for (size_t i = 0; i < NUM_KNOWN_KEYS; i++) {
        if (known_keys[i].kid_len == kid_len &&
            memcmp(known_keys[i].kid, kid, kid_len) == 0)
        {
            return known_keys[i].public_key;
        }
    }
    return NULL; // Nicht gefunden
}

int extract_kid_from_unprotected(cose_signature_dec_t *sig,
                                 uint8_t *kid_buf, size_t *kid_len)
{
    CborParser parser;
    CborValue root_array;

    if (cbor_parser_init(sig->buf, sig->len, 0, &parser, &root_array) != CborNoError)
        return KID_ERR_CBOR_PARSER_INIT;

    if (!cbor_value_is_array(&root_array))
        return KID_ERR_NOT_ARRAY;

    CborValue element;
    if (cbor_value_enter_container(&root_array, &element) != CborNoError)
        return KID_ERR_ENTER_ARRAY_FAILED;

    if (cbor_value_advance(&element) != CborNoError)
        return KID_ERR_ADVANCE_TO_UNPROT;

    if (!cbor_value_is_map(&element))
        return KID_ERR_UNPROT_NOT_MAP;

    CborValue map_it;
    if (cbor_value_enter_container(&element, &map_it) != CborNoError)
        return KID_ERR_ENTER_MAP_FAILED;

    while (!cbor_value_at_end(&map_it))
    {
        int64_t key;
        if (!cbor_value_is_integer(&map_it) || cbor_value_get_int64(&map_it, &key) != CborNoError)
            return KID_ERR_INVALID_KEY;

        if (cbor_value_advance(&map_it) != CborNoError)
            return KID_ERR_ADVANCE_TO_VALUE;

        if (key == 4 && cbor_value_is_byte_string(&map_it))
        {
            cbor_value_calculate_string_length(&map_it, kid_len);
            if (cbor_value_copy_byte_string(&map_it, kid_buf, kid_len, NULL) == CborNoError)
                return KID_OK;
            else
                return KID_ERR_COPY_KID_FAILED;
        }

        if (cbor_value_advance(&map_it) != CborNoError)
            return KID_ERR_ADVANCE_MAP_NEXT;
    }

    return KID_ERR_KID_NOT_FOUND;
}


int verify_decode(uint8_t *encodedMsg, size_t len, uint8_t *outbuf,
                  size_t outbuf_len, size_t *payload_len)
{
    if (cose_sign_decode(&verify, encodedMsg, len) != 0)
    {
        return COSE_VERIFY_ERR_DECODE_FAILED;
    }

    cose_signature_dec_t vsignature;

    // Erst Initialisieren der Iteration
    cose_sign_signature_iter_init(&vsignature);

    cose_sign_signature_iter(&verify, &vsignature);

    uint8_t kid[MAX_KEY_ID_SIZE];
    size_t kid_len = sizeof(kid);

    if (extract_kid_from_unprotected(&vsignature, kid, &kid_len) != COSE_SIGN_SUCCESS)
    {
        return COSE_VERIFY_ERR_KID_EXTRACTION_FAIL;
    }

    cose_key_init(&verifyer);

    const uint8_t *pubkey = get_public_key_for_kid(kid, kid_len);
    if (!pubkey) {
        return COSE_VERIFY_ERR_KID_UNKNOWN;
    }

    cose_key_set_keys(&verifyer,
                      COSE_EC_CURVE_ED25519,
                      COSE_ALGO_EDDSA,
                      (uint8_t *) pubkey,
                      NULL,
                      NULL);

    if (cose_sign_verify(&verify, &vsignature, &verifyer, v_buf, sizeof(v_buf)) != 0)
    {
        return COSE_VERIFY_ERR_VERIFY_FAIL;
    }

    if (verify.payload != NULL && verify.payload_len > 0)
    {
        if (verify.payload_len > outbuf_len)
        {
            return COSE_VERIFY_ERR_BUFFER_TOO_SMALL;
        }
        memcpy(outbuf, verify.payload, verify.payload_len);
        *payload_len = verify.payload_len;
    }
    else
    {
        *payload_len = 0;
    }

    return COSE_VERIFY_SUCCESS;
}





