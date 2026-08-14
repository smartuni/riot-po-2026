#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "od.h"
#include "flashdb_store_service.h"
#include "store_service.h"
#include "credential_manager.h"
#include "identity_store.h"
#if IS_USED(MODULE_FLASHDB_VFS)
#include "vfs_default.h"
#endif
#define LOG_LEVEL   LOG_INFO
#include "log.h"
#define _LOGDBG(...) LOG_DEBUG("[credential_manager_setup]: " __VA_ARGS__)
#define _LOGINF(...) LOG_INFO("[credential_manager_setup]: " __VA_ARGS__)
#define _LOGERR(...) LOG_ERROR("[credential_manager_setup]: " __VA_ARGS__)

static flashdb_store_service_ctx_t store_ctx;

static store_service_t store_service = {
    .interface = &flashdb_store_service_interface,
    .context = &store_ctx
};

int credential_manager_setup(const char *db_path) {
    int err;
#if IS_USED(MODULE_FLASHDB_VFS)
    /* Create the DB directory */
    err = vfs_mkdir(db_path, 0777);
    if (err != 0 && err != -EEXIST) {
        _LOGERR("Could not create the directory");
        printf("Error %d\n", err);
        return -1;
    }
#endif

    err = flashdb_store_service_init(&store_ctx, "cred_db", db_path);
    if (err) {
        _LOGERR("flashdb_store_service_init [FAILED]\n");
        return -2;
    }

    err = credential_manager_init(&store_service);
    if (err) {
        _LOGERR("credential_manager_init init [FAILED]\n");
        return -3;
    }

    identity_t root_identity;
    err = get_root_identity(&root_identity);
    if (err) {
        _LOGERR("getting root identity [FAILED]\n");
        return -5;
    }

    err = credential_manager_add_key(root_identity.kid, sizeof(root_identity.kid), CREDENTIAL_PUBLIC,
            root_identity.key, sizeof(root_identity.key));
    if (err) {
        _LOGERR("add root public key [FAILED]\n");
        return -4;
    }

    identity_t own_private_identity;
    err = get_own_private_identity(&own_private_identity);
    if (err) {
        _LOGERR("getting own private identity [FAILED]\n");
        return -5;
    }

    err = credential_manager_add_key(own_private_identity.kid, sizeof(own_private_identity.kid), CREDENTIAL_PRIVATE,
            own_private_identity.key, sizeof(own_private_identity.key));
    if (err) {
        _LOGERR("add own private key [FAILED]\n");
        return -4;
    }

    identity_t own_public_identity;
    err = get_own_public_identity(&own_public_identity);
    if (err) {
        _LOGERR("getting own public identity [FAILED]\n");
        return -5;
    }

    err = credential_manager_add_key(own_public_identity.kid, sizeof(own_public_identity.kid), CREDENTIAL_PUBLIC,
            own_public_identity.key, sizeof(own_public_identity.key));
    if (err) {
        _LOGERR("add own public key [FAILED]\n");
        return -4;
    }

    vfs_DIR dirp;
    err = get_public_identities_init(&dirp);
    if (err) {
        _LOGERR("get_public_identities_init [FAILED]\n");
        return -6;
    }

    int res;
    identity_t identity;
    while((res = get_public_identities_next(&dirp, &identity))) {
        if(res < 0) {
            _LOGERR("error getting next signed public identity, skipping\n");
            continue;
        }

        int err = credential_manager_add_key(identity.kid, sizeof(identity.kid), CREDENTIAL_PUBLIC, identity.key, sizeof(identity.key));
        if (err) {
            _LOGERR("add public key [FAILED]\n");
            return -5;
        }
        else if (LOG_LEVEL == LOG_DEBUG) {
            _LOGDBG("added key:\n");
            od_hex_dump(identity.kid, sizeof(identity.kid), 0);
        }
    }

    return 0;
}
