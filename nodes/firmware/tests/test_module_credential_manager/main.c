#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include "unity.h"
#include "flashdb_store_service.h"
#include "store_service.h"
#include "fal_cfg.h"
#include "vfs_default.h"

#include "credential_manager.h"

static uint8_t ed25519_secret_key[] = {
  0x4e, 0xd7, 0x4e, 0x9d, 0x1d, 0x45, 0x9e, 0xe9, 0x85, 0x33, 0xa0, 0x95, 0xc6, 0xb3,
  0x59, 0xcd, 0x00, 0x87, 0xfc, 0x14, 0xa7, 0x5f, 0x7b, 0x8b, 0x58, 0x7d, 0x50, 0xaf,
  0x85, 0x3b, 0xca, 0x87
};

static uint8_t ed25519_public_key[] = {
  0xa8, 0xc6, 0x51, 0xd4, 0xa2, 0xa7, 0xd9, 0x09, 0x54, 0x10, 0x51, 0x30, 0xa7, 0x4c,
  0x12, 0x0a, 0x3f, 0x9d, 0x2d, 0xf2, 0xde, 0x57, 0x76, 0x09, 0x3b, 0xb5, 0x90, 0xa0,
  0xc0, 0xd0, 0xdc, 0x08
};

static uint8_t own_kid[] = {0xDE, 0xAB, 0x12, 0x34};

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
}

void tearDown(void){}

static void _test_add_get_private_key(void)
{
    flashdb_store_service_ctx_t store_ctx;
    TEST_ASSERT_EQUAL_INT(0,
        flashdb_store_service_init(&store_ctx, "credentials_db", DB_DIR)
    );

    store_service_t store_service = {
        .interface = flashdb_store_service_interface,
        .context = &store_ctx
    };
    TEST_ASSERT_EQUAL_INT(0, credential_manager_init(&store_service));

    TEST_ASSERT_EQUAL_INT(0,
        credential_manager_add_key(own_kid, sizeof(own_kid), CREDENTIAL_PRIVATE,
                                   ed25519_secret_key, sizeof(ed25519_secret_key))
    );

    size_t key_len;
    uint8_t key[sizeof(ed25519_secret_key)];
    TEST_ASSERT_EQUAL_INT(0,
        credential_manager_get_key(own_kid, sizeof(own_kid), CREDENTIAL_PRIVATE, NULL,
                                   &key_len)
    );
    TEST_ASSERT_EQUAL_INT(sizeof(ed25519_secret_key), key_len);
    TEST_ASSERT_EQUAL_INT(0,
        credential_manager_get_key(own_kid, sizeof(own_kid), CREDENTIAL_PRIVATE, key,
                                   &key_len)
    );

    TEST_ASSERT_EQUAL_MEMORY(ed25519_secret_key, key, key_len);

    TEST_ASSERT_EQUAL_INT(0,
        credential_manager_delete_key(own_kid, sizeof(own_kid), CREDENTIAL_PRIVATE)
    );

    TEST_ASSERT_EQUAL_INT(-1,
        credential_manager_get_key(own_kid, sizeof(own_kid), CREDENTIAL_PRIVATE, key,
                                   &key_len)
    );
}

static void _test_add_get_public_key(void)
{
    flashdb_store_service_ctx_t store_ctx;
    TEST_ASSERT_EQUAL_INT(0,
        flashdb_store_service_init(&store_ctx, "credentials_db", DB_DIR)
    );

    store_service_t store_service = {
        .interface = flashdb_store_service_interface,
        .context = &store_ctx
    };
    TEST_ASSERT_EQUAL_INT(0, credential_manager_init(&store_service));

    TEST_ASSERT_EQUAL_INT(0,
        credential_manager_add_key(own_kid, sizeof(own_kid), CREDENTIAL_PUBLIC,
                                   ed25519_public_key, sizeof(ed25519_public_key))
    );

    size_t key_len;
    uint8_t key[sizeof(ed25519_public_key)];
    TEST_ASSERT_EQUAL_INT(0,
        credential_manager_get_key(own_kid, sizeof(own_kid), CREDENTIAL_PUBLIC, NULL,
                                   &key_len)
    );
    TEST_ASSERT_EQUAL_INT(sizeof(ed25519_public_key), key_len);
    TEST_ASSERT_EQUAL_INT(0,
        credential_manager_get_key(own_kid, sizeof(own_kid), CREDENTIAL_PUBLIC, key,
                                   &key_len)
    );
    TEST_ASSERT_EQUAL_MEMORY(ed25519_public_key, key, key_len);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(_test_add_get_private_key);
    RUN_TEST(_test_add_get_public_key);
    return UNITY_END();
}
