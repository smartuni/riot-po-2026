#define MOUNT_POINT "/mtdRAM"
#define DB_DIR MOUNT_POINT "/fdb_kvdb"

#include "mtd.h"
#include "mtd_emulated.h"
#include "fs/littlefs2_fs.h"
#include "vfs.h"

#define LOG_LEVEL LOG_DEBUG
#include "log.h"

#ifndef SECTOR_COUNT
#define SECTOR_COUNT (16)
#endif

#ifndef PAGE_PER_SECTOR
#define PAGE_PER_SECTOR (8)
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE (64)
#endif

MTD_EMULATED_DEV(0, SECTOR_COUNT, PAGE_PER_SECTOR, PAGE_SIZE);
#define MTD_SIZE_BYTES (PAGE_SIZE * PAGE_PER_SECTOR * SECTOR_COUNT)

#define mtd_ram_dev (&mtd_emulated_dev0.base)

static littlefs2_desc_t littlefs_desc;

static vfs_mount_t _littlefs_mount = {
    .fs = &littlefs2_file_system,
    .private_data = &littlefs_desc,
};

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

int storage_setup_ram_mtd(const char *mount_path)
{
    littlefs_desc.dev = mtd_ram_dev;
    _littlefs_mount.mount_point = mount_path;
    int err = mtd_init(mtd_ram_dev);
    LOG_DEBUG("%s: mtd_init: %s\n", __func__, ok(!err));
    if (err) {
        return -1;
    }

    err = mtd_erase(mtd_ram_dev, 0, MTD_SIZE_BYTES);
    LOG_DEBUG("%s: mtd_erase: %s\n", __func__, ok(!err));
    if (err) {
        return -2;
    }

    err = vfs_format(&_littlefs_mount);
    LOG_DEBUG("%s: vfs_format: %s\n", __func__, ok(!err));
    if (err) {
        return -3;
    }

    err = vfs_mount(&_littlefs_mount);
    LOG_DEBUG("%s: vfs_mount: %s\n", __func__, ok(!err));
    if (err) {
        return -4;
    }

    return 0;
}

mtd_dev_t *storage_setup_get_ram_mtd(void)
{
    return mtd_ram_dev;
}
