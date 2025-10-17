#include "unity.h"

#include "embedDB.h"
#include "embeddb_vfs_interface.h"

#include "fs/littlefs_fs.h"
#include "vfs.h"
#include "mtd.h"

#include <fcntl.h>

#define ENABLE_DEBUG 0
#include "debug.h"

#include "mtd_emulated.h"

/* Test mock object implementing a simple RAM-based mtd */
#ifndef SECTOR_COUNT
#define SECTOR_COUNT 12
#endif
#ifndef PAGE_PER_SECTOR
#define PAGE_PER_SECTOR 4
#endif
#ifndef PAGE_SIZE
#define PAGE_SIZE 64
#endif

MTD_EMULATED_DEV(0, SECTOR_COUNT, PAGE_PER_SECTOR, PAGE_SIZE);

#define _dev (&mtd_emulated_dev0.base)

#define MOUNT_PATH "/test-fs"
#define DB_PATH MOUNT_PATH "/embeddb.db"
#define DB_BUFFER_SIZE_PAGES 2

static littlefs_desc_t littlefs_desc;
static vfs_mount_t _test_littlefs_mount = {
    .fs = &littlefs_file_system,
    .mount_point = MOUNT_PATH,
    .private_data = &littlefs_desc,
};

embeddb_file_info_t db_file_info = {
    .filename = DB_PATH,
    .fd = -1,
};

uint8_t embeddb_buffer[DB_BUFFER_SIZE_PAGES * PAGE_SIZE];
embedDBState state;

void setUp(void)
{
    littlefs_desc.dev = _dev;
    vfs_format(&_test_littlefs_mount);
    vfs_mount(&_test_littlefs_mount);
}

void tearDown(void)
{
    vfs_unlink(DB_PATH);
    vfs_umount(&_test_littlefs_mount, false);
}

static void test_embeddb_create_db(void)
{

    state.keySize = 4;
    state.dataSize = 12;

    // Function pointers that can compare keys and data (user customizable)
    state.compareKey = int32Comparator;

    // File interface
    state.pageSize = PAGE_SIZE;
    state.eraseSizeInPages = 4;
    state.numDataPages = SECTOR_COUNT * PAGE_PER_SECTOR / 2;
    state.numIndexPages = SECTOR_COUNT * PAGE_PER_SECTOR / 2;
    state.fileInterface = &embeddb_vfs_file_interface;
    state.dataFile = &db_file_info;

    // Configure memory buffers
    state.bufferSizeInBlocks = DB_BUFFER_SIZE_PAGES; // Minimum 2 buffers is required for read/write operations
    state.buffer = embeddb_buffer;

    // Initialize
    TEST_ASSERT_EQUAL_INT(embedDBInit(&state, 1), 0);

    // Store record
    uint32_t key = 123;
    char data[12] = "TEST DATA";
    TEST_ASSERT_EQUAL_INT(embedDBPut(&state, (void*) &key, data), 0);

    char returnData[12];
    // Get record
    TEST_ASSERT_EQUAL_INT(embedDBGet(&state, (void*) &key, (void*) returnData), 0);

    TEST_ASSERT_EQUAL_STRING(data, returnData);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_embeddb_create_db);
    return UNITY_END();
}
