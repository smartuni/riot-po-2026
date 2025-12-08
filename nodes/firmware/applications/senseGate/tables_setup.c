#include <errno.h>

#include "hlc_ztimer.h"
#if IS_USED(MODULE_FLASHDB_VFS)
#include "vfs.h"
#include "vfs_default.h"
#endif
#include "hybrid_logical_clock.h"
#include "crypto_service.h"
#include "cose_crypto_service.h"
#include "store_service.h"
#include "flashdb_store_service.h"

#include "tables.h"
#include "tables/types.h"
#include "tables/keys.h"
#include "tables/records.h"
#include "personalization.h"

#define LOG_LEVEL LOG_DEBUG
#include "log.h"

static flashdb_store_service_ctx_t store_ctx;

static hlc_ctx_t hlc_ctx;
static tables_context_t tables;
static hlc_ztimer_t hlc_ztimer_ctx;

static store_service_t store_service = {
    .interface = &flashdb_store_service_interface,
    .context = &store_ctx
};

static cose_crypto_service_context_t crypto_ctx;

static crypto_service_t crypto_service = {
    .context = &crypto_ctx,
    .interface = &cose_crypto_service_interface,
};

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

int tables_setup(tables_context_t **t, const char *db_path)
{
    int err;
#if IS_USED(MODULE_FLASHDB_VFS)
    /* Create the DB directory */
    err = vfs_mkdir(db_path, 0777);
    if (err != 0 && err != -EEXIST) {
        puts("Could not create the directory");
        printf("Error %d\n", err);
        return -1;
    }
#endif

    err = flashdb_store_service_init(&store_ctx, "tables_db", db_path);
    LOG_DEBUG("%s: flashdb_store_service_init %s\n", __func__, ok(!err));
    if (err) {
        return -2;
    }

    err = cose_crypto_service_init(&crypto_ctx);
    LOG_DEBUG("%s: cose_crypto_service_init %s\n", __func__, ok(!err));
    if (err) {
        return -3;
    }

    err = hlc_ztimer_init(&hlc_ztimer_ctx, 0);
    LOG_DEBUG("%s: hlc_ztimer_init %s\n", __func__, ok(!err));
    if (err) {
        return -4;
    }

    err = hlc_init(&hlc_ctx, hlc_ztimer_get_time, &hlc_ztimer_ctx);
    LOG_DEBUG("%s: hlc_init %s\n", __func__, ok(!err));
    if (err) {
        return -5;
    }

    err = tables_init(&tables, &self_node_id, &store_service, &crypto_service, &hlc_ctx);
    LOG_DEBUG("%s: tables_init %s\n", __func__, ok(!err));
    if (err) {
        return -6;
    }

    *t = &tables;
    return 0;
}

