#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "od.h"
#include "personalization.h"
#include "flashdb_store_service.h"
#include "store_service.h"
#include "vfs.h"
#include "vfs_default.h"
#include "credential_manager.h"
#include "identity_store.h"
#define LOG_LEVEL   LOG_DEBUG
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

    identity_t root_pubid;
    identity_t self_prvid;
    identity_t self_pubid;

    get_root_identity(&root_pubid);

    printf("\n");
    od_hex_dump(root_pubid.kid, sizeof(root_pubid.kid), 0);
    printf("\n");
    od_hex_dump(root_pubid.key, sizeof(root_pubid.key), 0);
    printf("\n");

    err = credential_manager_add_key(root_pubid.kid, sizeof(root_pubid.kid), CREDENTIAL_PUBLIC,
            root_pubid.key, sizeof(root_pubid.key));
    if (err) {
        _LOGERR("add root public key [FAILED]");
        return -4;
    }

    get_private_identity(&self_prvid);

    printf("\n");
    od_hex_dump(self_prvid.kid, sizeof(self_prvid.kid), 0);
    printf("\n");
    od_hex_dump(self_prvid.key, sizeof(self_prvid.key), 0);
    printf("\n");

    err = credential_manager_add_key(self_prvid.kid, sizeof(self_prvid.kid), CREDENTIAL_PRIVATE,
            self_prvid.key, sizeof(self_prvid.key));
    if (err) {
        _LOGERR("add private key [FAILED]");
        return -4;
    }

    get_public_identity(&self_pubid);

    printf("\n");
    od_hex_dump(self_pubid.kid, sizeof(self_pubid.kid), 0);
    printf("\n");
    od_hex_dump(self_pubid.key, sizeof(self_pubid.key), 0);
    printf("\n");

    err = credential_manager_add_key(self_pubid.kid, sizeof(self_pubid.kid), CREDENTIAL_PUBLIC,
            self_pubid.key, sizeof(self_pubid.key));
    if (err) {
        _LOGERR("add self public key [FAILED]");
        return -4;
    }

    vfs_DIR dirp;
    int res = vfs_opendir(&dirp, IDENTITY_STORAGE_PATH "/valid");
    _LOGDBG("opendir result %d\n", res);
    if(res == 0) {
        vfs_dirent_t dirent;
        while((res = vfs_readdir(&dirp, &dirent)) == 1) {
            if(strcmp(dirent.d_name, ".") != 0 && strcmp(dirent.d_name, "..") != 0) {
                puts(dirent.d_name);
                uint8_t ed25519_public_key[32];
                read_identity(dirent.d_name, ed25519_public_key, 32);

                for (size_t i = 0; i < 32; i++) {
                    printf("0x%02x ", ed25519_public_key[i]);
                }
                printf("\n");

                char dev_type_value = 0xFF;
                char dev_id_value = 0xFF;
                if (memcmp("sensemate", dirent.d_name, strlen("sensemate")) == 0) {
                    dev_type_value = DEVICE_TYPE_SENSEMATE;
                    dev_id_value = atoi(&dirent.d_name[strlen("sensemate") + 1]);
                } else  if (memcmp("sensegate", dirent.d_name, strlen("sensegate")) == 0) {
                    dev_type_value = DEVICE_TYPE_GATE;
                    dev_id_value = atoi(&dirent.d_name[strlen("sensegate") + 1]);
                }

                node_id_t kid = { 0x00, 0x00, dev_type_value, dev_id_value};
                
                err = credential_manager_add_key(kid, sizeof(kid), CREDENTIAL_PUBLIC, ed25519_public_key, sizeof(ed25519_public_key));
                if (err) {
                    _LOGERR("add public key [FAILED]");
                    return -5;
                }
            }
        }
    }

    /*
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
    */

    return 0;
}

