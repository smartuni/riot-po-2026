/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     pkg_embeddb_vfs
 * @{
 * @file
 * @brief       Functions for the VFS File Interface for EmbedDB
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include "embedDB.h"
#include "embeddb_vfs_interface.h"

#include "vfs.h"

#include <fcntl.h>

int8_t _read(void *buffer, uint32_t page_num, uint32_t page_size, void *file)
{
    assert(buffer != NULL);
    assert(file != NULL);

    embeddb_file_info_t *f_info = (embeddb_file_info_t *) file;
    assert(f_info->fd >= 0);

    off_t offset = page_num * page_size;
    off_t result = vfs_lseek(f_info->fd, offset, SEEK_SET);
    if (result < 0) {
        return 0;
    }

    ssize_t read_bytes = vfs_read(f_info->fd, buffer, page_size);
    if (read_bytes != page_size) {
        return 0;
    }

    return 1;
}

int8_t _write(void *buffer, uint32_t page_num, uint32_t page_size, void *file)
{
    assert(buffer != NULL);
    assert(file != NULL);

    embeddb_file_info_t *f_info = (embeddb_file_info_t *) file;
    assert(f_info->fd >= 0);

    off_t offset = page_num * page_size;
    off_t result = vfs_lseek(f_info->fd, offset, SEEK_SET);
    if (result < 0) {
        return 0;
    }

    ssize_t written_bytes = vfs_write(f_info->fd, buffer, page_size);
    if (written_bytes != page_size) {
        return 0;
    }
    return 1;
}

int8_t _close(void *file)
{
    assert(file != NULL);
    embeddb_file_info_t *f_info = (embeddb_file_info_t *) file;
    assert(f_info->fd >= 0);

    int result = vfs_close(f_info->fd);
    if (result < 0) {
        return 0;
    }
    f_info->fd = -1;
    return 1;
}

int8_t _open(void *file, uint8_t mode)
{
    assert(file != NULL);
    embeddb_file_info_t *f_info = (embeddb_file_info_t *) file;
    assert(f_info->fd == -1);

    int vfs_flags;
    if (EMBEDDB_FILE_MODE_W_PLUS_B == mode) {
        vfs_flags = O_RDWR | O_CREAT;
    } else if (EMBEDDB_FILE_MODE_R_PLUS_B == mode) {
        vfs_flags = O_RDWR;
    } else {
        return 0;
    }

    int fd = vfs_open(f_info->filename, vfs_flags, 0);
    if (fd < 0) {
        return 0;
    }
    f_info->fd = fd;
    return 1;
}

int8_t _flush(void *file)
{
    assert(file != NULL);
    embeddb_file_info_t *f_info = (embeddb_file_info_t *) file;
    assert(f_info->fd >= 0);

    int result = vfs_fsync(f_info->fd);
    if (result < 0) {
        return 0;
    }
    return 1;
}


embedDBFileInterface embeddb_vfs_file_interface = {
    .open = _open,
    .close = _close,
    .read = _read,
    .write = _write,
    .flush = _flush,
};
