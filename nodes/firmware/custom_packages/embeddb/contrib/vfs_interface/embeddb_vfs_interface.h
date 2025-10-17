/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

#pragma once

/**
 * @defgroup    pkg_embeddb_vfs VFS File Interface for EmbedDB
 * @ingroup     pkg_embeddb
 * @brief       VFS File Interface for EmbedDB
 *
 * The VFS File Interface allows EmbedDB to interact with files using the
 * Virtual File System (VFS) abstraction layer.
 * @{
 *
 * @file
 * @brief       Definitions for the VFS File Interface for EmbedDB
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "embedDB.h"

/**
 * @brief Structure to hold file information for EmbedDB VFS interface
 *
 * This structure has to be initialized and passed to the embedDBState struct in the
 * `dataFile` field, during the initialization of EmbedDB.
 */
typedef struct {
    const char *filename; /**< Name of the file to use as database */
    int fd;               /**< File descriptor */
} embeddb_file_info_t;

/**
 * @brief VFS File Interface for EmbedDB
 *
 * This interface provides function pointers for file operations
 * using the VFS abstraction layer. It has to be assigned to the
 * `fileInterface` field of the embedDBState struct during the
 * initialization of EmbedDB.
 */
extern embedDBFileInterface embeddb_vfs_file_interface;

#ifdef __cplusplus
}
#endif

/** @} */
