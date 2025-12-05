#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include "unity.h"
#include "fal_cfg.h"
#include "vfs_default.h"

#include "flashdb_store_service.h"
#include "credential_manager.h"
#include "cose_crypto_service.h"
#include "crypto_service.h"

static uint8_t ed25519_secret_key[] = {
    0x74, 0x65, 0x73, 0x74, 0x5f, 0x73, 0x65, 0x65, 0x64, 0x5f, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x30, 0x31, 0x32
};

static uint8_t ed25519_public_key[] = {
    0xd5, 0x38, 0x03, 0x54, 0x43, 0xa5, 0x0c, 0xff, 0xf8, 0x19, 0xc6, 0xdc, 0xc7, 0x59,
    0x75, 0x46, 0xbc, 0x05, 0x96, 0xca, 0x32, 0xb2, 0xbb, 0xc7, 0x43, 0xac, 0xdd, 0x1c,
    0xa7, 0x1b, 0xcb, 0x45
};

static uint8_t own_kid[] = {0xDE, 0xAB, 0x12, 0x34};

static flashdb_store_service_ctx_t store_ctx;
static store_service_t store_service;

static cose_crypto_service_context_t crypto_ctx;
static crypto_service_t crypto_service;

#define DB_DIR VFS_DEFAULT_DATA "/credentials"
void setUp(void)
{
    /* Create the DB directory */
    int err = vfs_mkdir(DB_DIR, 0777);
    if (err != 0 && err != -EEXIST) {
        puts("Could not create the directory");
        printf("Error %d\n", err);
        TEST_ASSERT(false);
    }

    TEST_ASSERT_EQUAL_INT(0,
        flashdb_store_service_init(&store_ctx, "credentials_db", DB_DIR)
    );

    store_service.interface = &flashdb_store_service_interface;
    store_service.context = &store_ctx;

    TEST_ASSERT_EQUAL_INT(0, credential_manager_init(&store_service));

    TEST_ASSERT_EQUAL_INT(0,
        credential_manager_add_key(own_kid, sizeof(own_kid), CREDENTIAL_PRIVATE,
                                   ed25519_secret_key, sizeof(ed25519_secret_key))
    );

    TEST_ASSERT_EQUAL_INT(0,
        credential_manager_add_key(own_kid, sizeof(own_kid), CREDENTIAL_PUBLIC,
                                   ed25519_public_key, sizeof(ed25519_public_key))
    );
}

void tearDown(void){}

static void test_cose_crypto_service_init(void)
{
    TEST_ASSERT_EQUAL_INT(0, cose_crypto_service_init(&crypto_ctx));
    crypto_service.context = &crypto_ctx;
    crypto_service.interface = &cose_crypto_service_interface;
}

static void test_sign_verify(void)
{
    const uint8_t data[] = {0xFF, 0xEE, 0xDD, 0xAA, 0x55, 0x33};
    size_t sign_len;
    TEST_ASSERT_EQUAL_INT(0,
        crypto_service_sign(&crypto_service, own_kid, sizeof(own_kid), data,
                            sizeof(data), NULL, &sign_len)
    );

    uint8_t sign[sign_len];
    TEST_ASSERT_EQUAL_INT(0,
        crypto_service_sign(&crypto_service, own_kid, sizeof(own_kid), data,
                            sizeof(data), sign, &sign_len)
    );

    TEST_ASSERT_EQUAL_INT(0,
        crypto_service_verify(&crypto_service, own_kid, sizeof(own_kid), data,
                              sizeof(data), sign, sign_len)
    );
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_cose_crypto_service_init);
    RUN_TEST(test_sign_verify);
    (void)ed25519_public_key;
    return UNITY_END();
}
