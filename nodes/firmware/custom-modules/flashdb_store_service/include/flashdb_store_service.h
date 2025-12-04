#pragma once

#include "mutex.h"
#include "flashdb.h"
#include "store_service.h"

/**
 * @brief Context for the FlashDB store service
 */
typedef struct {
    /**< FlashDB KVDB instance */
    struct fdb_kvdb kvdb;

    /**< Mutex to protect access to the context */
    mutex_t fdb_lock;

    /**< Mutex to protect service operations */
    mutex_t service_lock;
} flashdb_store_service_ctx_t;

typedef struct __attribute__((aligned(__alignof__(store_service_iterator_t)))) {
    struct fdb_kv_iterator iterator;
    store_service_query_t query;
} flashdb_store_service_iterator_t;

/**
 * @brief Size of each sector in the FlashDB store service
 */
#ifndef FLASHDB_STORE_SERVICE_SECTOR_SIZE
#define FLASHDB_STORE_SERVICE_SECTOR_SIZE (4096)
#endif

/**
 * @brief Total size of the FlashDB store service database
 */
#ifndef FLASHDB_STORE_SERVICE_DB_SIZE
#define FLASHDB_STORE_SERVICE_DB_SIZE (FLASHDB_STORE_SERVICE_SECTOR_SIZE * 4)
#endif

/**
 * @brief Initialize the FlashDB store service
 * @param ctx       Pointer to the FlashDB store service context to initialize
 * @param db_name   Name of the FlashDB database
 * @param db_dir    Directory where the FlashDB database files will be stored
 *
 * @retval 0 on success
 * @retval <0 on error
 *
 * @note @p db_name and @p db_dir will be copied, no need to keep them valid after.
 */
int flashdb_store_service_init(flashdb_store_service_ctx_t *ctx, const char *db_name,
                               const char *db_dir);

/**
 * @brief FlashDB tables store service interface
 */
extern const store_service_interface_t flashdb_store_service_interface;
