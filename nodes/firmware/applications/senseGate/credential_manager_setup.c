#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "od.h"
#include "personalization.h"
#include "flashdb_store_service.h"
#include "store_service.h"
#if IS_USED(MODULE_FLASHDB_VFS)
#include "vfs_default.h"
#endif
#include "credential_manager.h"
#include "key_config.h"
#include "secrets/public_keys.h"
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
        _LOGERR("flashdb_store_service_init [FAILED]");
        return -2;
    }

    err = credential_manager_init(&store_service);
    if (err) {
        _LOGERR("credential_manager_init init [FAILED]");
        return -3;
    }

    err = credential_manager_add_key(self_node_id, sizeof(self_node_id), CREDENTIAL_PRIVATE,
            ed25519_secret_key, sizeof(ed25519_secret_key));
    if (err) {
        _LOGERR("add private key [FAILED]");
        return -4;
    }

    // public key of the key_config header is added from the known keys already
    (void)ed25519_public_key;

    for (unsigned i = 0; i < ARRAY_SIZE(known_keys); i++) {
        const ed25519_public_key_entry_t *key = &known_keys[i];
        const uint8_t *legacy_kid = known_keys[i].kid;

        // TDOD: replace this remapping. The key-distro credential generator script should just
        //       create the new key id format directly.
        char dev_type_value = 0xFF;
        char dev_id_value = 0xFF;
        if (memcmp("sensemate", legacy_kid, strlen("sensemate")) == 0) {
            dev_type_value = DEVICE_TYPE_SENSEMATE;
            dev_id_value = atoi((char*)&legacy_kid[strlen("sensemate") + 1]);
        } else  if (memcmp("sensegate", legacy_kid, strlen("sensegate")) == 0) {
            dev_type_value = DEVICE_TYPE_GATE;
            dev_id_value = atoi((char*)&legacy_kid[strlen("sensegate") + 1]);
        }

        const node_id_t new_kid = { 0x00, 0x00, dev_type_value, dev_id_value};

        err = credential_manager_add_key(new_kid, sizeof(new_kid), CREDENTIAL_PUBLIC,
                key->public_key, sizeof(key->public_key));
        if (err) {
            _LOGERR("add public key [FAILED]");
            return -5;
        }
    }
    return 0;
}

