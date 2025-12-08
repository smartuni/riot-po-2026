#define MOUNT_POINT "/mtdRAM"
#define DB_DIR MOUNT_POINT "/fdb_kvdb"

#include "mtd.h"
#include "mtd_emulated.h"
#if IS_USED(MODULE_FLASHDB_VFS)
#include "fs/littlefs2_fs.h"
#include "vfs.h"
#endif

#define LOG_LEVEL LOG_DEBUG
#include "log.h"

#ifndef RAM_MTD_SECTOR_COUNT
#define RAM_MTD_SECTOR_COUNT (16)
#endif

#ifndef RAM_MTD_PAGE_PER_SECTOR
#define RAM_MTD_PAGE_PER_SECTOR (8)
#endif

#ifndef RAM_MTD_PAGE_SIZE
#define RAM_MTD_PAGE_SIZE (64)
#endif

MTD_EMULATED_DEV(0, RAM_MTD_SECTOR_COUNT, RAM_MTD_PAGE_PER_SECTOR, RAM_MTD_PAGE_SIZE);
#define MTD_SIZE_BYTES (RAM_MTD_SECTOR_COUNT * RAM_MTD_PAGE_PER_SECTOR * RAM_MTD_PAGE_SIZE)

#define mtd_ram_dev (&mtd_emulated_dev0.base)

#if IS_USED(MODULE_FLASHDB_VFS)
static littlefs2_desc_t littlefs_desc;

static vfs_mount_t _littlefs_mount = {
    .fs = &littlefs2_file_system,
    .private_data = &littlefs_desc,
};
#endif

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

extern void fdb_mtd_init(mtd_dev_t *mtd);

int storage_setup_ram_mtd(const char *mount_path)
{
    int err;
#if IS_USED(MODULE_FLASHDB_VFS)
    littlefs_desc.dev = mtd_ram_dev;
    _littlefs_mount.mount_point = mount_path;
    err = mtd_init(mtd_ram_dev);
    LOG_DEBUG("%s: mtd_init: %s\n", __func__, ok(!err));
    if (err) {
        return -1;
    }
#else
    (void)mount_path;
    fdb_mtd_init(mtd_ram_dev);
#endif

    err = mtd_erase(mtd_ram_dev, 0, MTD_SIZE_BYTES);
    LOG_DEBUG("%s: mtd_erase: %s\n", __func__, ok(!err));
    if (err) {
        return -2;
    }

#if IS_USED(MODULE_FLASHDB_VFS)
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
#endif

    return 0;
}

mtd_dev_t *storage_setup_get_ram_mtd(void)
{
    return mtd_ram_dev;
}
