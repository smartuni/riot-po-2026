#include <errno.h>
#include "fal_cfg.h"
#include "vfs_default.h"
#include "unity.h"

#include "flashdb_store_service.h"
#include "store_service.h"
#include "ztimer.h"

#define MOUNT_POINT "/mtdRAM"
#define DB_DIR MOUNT_POINT "/fdb_kvdb"

#include "vfs.h"
#include "vfs_util.h"

#include "mtd_emulated.h"

MTD_EMULATED_DEV(0, SECTOR_COUNT, PAGE_PER_SECTOR, PAGE_SIZE);
#define MTD_SIZE_BYTES (PAGE_SIZE * PAGE_PER_SECTOR * SECTOR_COUNT)

#define mtd_dev (&mtd_emulated_dev0.base)

static littlefs2_desc_t littlefs_desc;

static vfs_mount_t _test_littlefs_mount = {
    .fs = &littlefs2_file_system,
    .mount_point = MOUNT_POINT,
    .private_data = &littlefs_desc,
};

void setUp(void)
{
    littlefs_desc.dev = mtd_dev;
    TEST_ASSERT_EQUAL(0, mtd_init(mtd_dev));
    TEST_ASSERT_EQUAL(0, mtd_erase(mtd_dev, 0, MTD_SIZE_BYTES));
    TEST_ASSERT_EQUAL(0, vfs_format(&_test_littlefs_mount));
    TEST_ASSERT_EQUAL(0, vfs_mount(&_test_littlefs_mount));

    /* Create the DB directory */
    int err = vfs_mkdir(DB_DIR, 0777);

    if (err != 0 && err != -EEXIST) {
        puts("Could not create the directory");
        printf("Error %d\n", err);
        TEST_ASSERT(false);
    }
}

void tearDown(void)
{
    TEST_ASSERT_EQUAL_INT(0,
        vfs_umount(&_test_littlefs_mount, true)
    );
}

static void _test_put_get_delete(void)
{
    uint8_t key[] = {0x01, 0x02, 0x03, 0x04};
    size_t key_len = sizeof(key);
    uint8_t key2[] = {0x01, 0x02, 0x04, 0x05};
    size_t key2_len = sizeof(key2);
    uint8_t key3[] = {0x01, 0x02, 0x06, 0x07};
    size_t key3_len = sizeof(key3);
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    size_t data_len = sizeof(data);

    flashdb_store_service_ctx_t context;
    store_service_t service = {
        .context = &context,
        .interface = &flashdb_store_service_interface
    };

    int res = flashdb_store_service_init(&context, "db_1", DB_DIR);

    TEST_ASSERT_EQUAL_INT(0, res);

    TEST_ASSERT_EQUAL_INT(0,store_service_put(&service, key, key_len, data, data_len));

    TEST_ASSERT_EQUAL_INT(0,store_service_put(&service, key2, key2_len, data, data_len));

    TEST_ASSERT_EQUAL_INT(0,store_service_put(&service, key3, key3_len, data, data_len));

    uint8_t retrieved_data[data_len];
    TEST_ASSERT_EQUAL_INT(0,
        store_service_get(&service, key, key_len, retrieved_data, data_len)
    );
    TEST_ASSERT_EQUAL_MEMORY(data, retrieved_data, data_len);

    TEST_ASSERT_EQUAL_INT(0,
        store_service_get(&service, key2, key2_len, retrieved_data, data_len)
    );
    TEST_ASSERT_EQUAL_MEMORY(data, retrieved_data, data_len);

    TEST_ASSERT_EQUAL_INT(0,
        store_service_get(&service, key3, key3_len, retrieved_data, data_len)
    );
    TEST_ASSERT_EQUAL_MEMORY(data, retrieved_data, data_len);

    TEST_ASSERT_EQUAL_INT(0, store_service_delete(&service, key, key_len));

    TEST_ASSERT_EQUAL_INT(-1,
        store_service_get(&service, key, key_len, retrieved_data, data_len)
    );
}

static void _test_iterator(void)
{
    uint8_t key1[] = {0xAA, 0xBB, 0x00, 0x00, 0x01};
    uint8_t key2[] = {0xAA, 0xBC, 0x00, 0x00, 0x02};
    uint8_t key3[] = {0xAA, 0xBB, 0x00, 0x01, 0x03};

    uint8_t data[] = {0xFF};

    flashdb_store_service_ctx_t context;
    store_service_t service = {
        .context = &context,
        .interface = &flashdb_store_service_interface
    };

    TEST_ASSERT_EQUAL_INT(0,
        flashdb_store_service_init(&context, "db_2", DB_DIR)
    );

    TEST_ASSERT_EQUAL_INT(0,
        store_service_put(&service, key1, sizeof(key1), data, sizeof(data))
    );
    TEST_ASSERT_EQUAL_INT(0,
       store_service_put(&service, key2, sizeof(key2), data, sizeof(data))
    );
    TEST_ASSERT_EQUAL_INT(0,
        store_service_put(&service, key3, sizeof(key3), data, sizeof(data))
    );

    STORE_ITERATOR(iterator, &service);

    TEST_ASSERT_EQUAL_INT(
        alignof(store_service_iterator_t),
        alignof(flashdb_store_service_iterator_t)
    );

    uint8_t filter1[] = {0xAA, 0xB0, 0x00, 0x00, 0x00};
    uint8_t mask1[] = {0xFF, 0xF0, 0x00, 0x00, 0x00};

    store_service_query_t query = {
        .key=filter1,
        .key_len=sizeof(filter1),
        .key_mask=mask1
    };

    TEST_ASSERT_EQUAL_INT(0, store_service_iterator_init(&service, iterator, &query));

    uint8_t key[5];
    size_t key_len = sizeof(key);
    uint8_t recovered_data[1];
    size_t data_len = sizeof(recovered_data);

    // Should match 3 times
    TEST_ASSERT_EQUAL_INT(0,
        store_service_iterator_next(&service, iterator, key, &key_len, recovered_data,
                                    &data_len)
    );

    TEST_ASSERT_EQUAL_size_t(sizeof(key1), key_len);
    TEST_ASSERT_EQUAL_MEMORY(key1, key, sizeof(key1));
    TEST_ASSERT_EQUAL_size_t(sizeof(data), data_len);
    TEST_ASSERT_EQUAL_MEMORY(data, recovered_data, sizeof(data));

    TEST_ASSERT_EQUAL_INT(0,
        store_service_iterator_next(&service, iterator, key, &key_len, recovered_data,
                                    &data_len)
    );
    TEST_ASSERT_EQUAL_size_t(sizeof(key2), key_len);
    TEST_ASSERT_EQUAL_MEMORY(key2, key, sizeof(key2));
    TEST_ASSERT_EQUAL_size_t(sizeof(data), data_len);
    TEST_ASSERT_EQUAL_MEMORY(data, recovered_data, sizeof(data));

    TEST_ASSERT_EQUAL_INT(0,
        store_service_iterator_next(&service, iterator, key, &key_len, recovered_data,
                                    &data_len)
    );
    TEST_ASSERT_EQUAL_size_t(sizeof(key3), key_len);
    TEST_ASSERT_EQUAL_MEMORY(key3, key, sizeof(key3));
    TEST_ASSERT_EQUAL_size_t(sizeof(data), data_len);
    TEST_ASSERT_EQUAL_MEMORY(data, recovered_data, sizeof(data));

    // No more matches
    TEST_ASSERT_EQUAL_INT(-1,
        store_service_iterator_next(&service, iterator, key, &key_len, recovered_data,
                                    &data_len)
    );

    uint8_t filter2[] = {0xAA, 0xBB, 0x00, 0x00, 0x00};
    uint8_t mask2[] = {0xFF, 0xFF, 0x00, 0x00, 0x00};
    query.key = filter2;
    query.key_mask = mask2;

    TEST_ASSERT_EQUAL_INT(0, store_service_iterator_init(&service, iterator, &query));

    // Should match 2 times
    TEST_ASSERT_EQUAL_INT(0,
        store_service_iterator_next(&service, iterator, key, &key_len, recovered_data,
                                    &data_len)
    );
    TEST_ASSERT_EQUAL_size_t(sizeof(key1), key_len);
    TEST_ASSERT_EQUAL_MEMORY(key1, key, sizeof(key1));
    TEST_ASSERT_EQUAL_size_t(sizeof(data), data_len);
    TEST_ASSERT_EQUAL_MEMORY(data, recovered_data, sizeof(data));

    TEST_ASSERT_EQUAL_INT(0,
        store_service_iterator_next(&service, iterator, key, &key_len, recovered_data,
                                    &data_len)
    );
    TEST_ASSERT_EQUAL_size_t(sizeof(key3), key_len);
    TEST_ASSERT_EQUAL_MEMORY(key3, key, sizeof(key3));
    TEST_ASSERT_EQUAL_size_t(sizeof(data), data_len);
    TEST_ASSERT_EQUAL_MEMORY(data, recovered_data, sizeof(data));

    // No more matches
    TEST_ASSERT_EQUAL_INT(-1,
        store_service_iterator_next(&service, iterator, key, &key_len, recovered_data,
                                    &data_len)
    );
}

int main(void)
{
    ztimer_sleep(ZTIMER_MSEC, 3000);
    UNITY_BEGIN();
    RUN_TEST(_test_put_get_delete);
    RUN_TEST(_test_iterator);
    return UNITY_END();
}
