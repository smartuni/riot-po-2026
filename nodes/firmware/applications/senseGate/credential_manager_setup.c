#include <errno.h>
#include "personalization.h"
#include "flashdb_store_service.h"
#include "store_service.h"
#include "vfs_default.h"
#include "credential_manager.h"
#include "key_config.h"

static flashdb_store_service_ctx_t store_ctx;

static store_service_t store_service = {
    .interface = &flashdb_store_service_interface,
    .context = &store_ctx
};

int credential_manager_setup(const char *db_path) {
    /* Create the DB directory */
    int err = vfs_mkdir(db_path, 0777);
    if (err != 0 && err != -EEXIST) {
        puts("Could not create the directory");
        printf("Error %d\n", err);
        return -1;
    }

    err = flashdb_store_service_init(&store_ctx, "cred_db", db_path);
    if (err) {
        printf("credential_manager_setup flashdb_store_service_init [FAILED]");
        return -2;
    }

    err = credential_manager_init(&store_service);
    if (err) {
        printf("credential_manager_setup credential_manager_init [FAILED]");
        return -3;
    }

    err = credential_manager_add_key(self_node_id, sizeof(self_node_id), CREDENTIAL_PRIVATE,
            ed25519_secret_key, sizeof(ed25519_secret_key));
    if (err) {
        printf("credential_manager_setup add private key [FAILED]");
        return -4;
    }

    err = credential_manager_add_key(self_node_id, sizeof(self_node_id), CREDENTIAL_PUBLIC,
            ed25519_public_key, sizeof(ed25519_public_key));
    if (err) {
        printf("credential_manager_setup add public key [FAILED]");
        return -5;
    }
    return 0;
}

