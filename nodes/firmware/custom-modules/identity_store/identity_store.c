#include <stdio.h>
#include <fcntl.h>

#include "vfs.h"
#include "vfs_default.h"
#include "vfs_util.h"
#include "identity_store.h"
#include "base64.h"
#include "od.h"
#include "cbor_serialization.h"
#include "errno.h"
#include "shell.h"

#define LOG_LEVEL LOG_INFO
#include "log.h"
#define _LOGDBG(...) LOG_DEBUG("[identity_store]: " __VA_ARGS__)
#define _LOGINF(...) LOG_INFO("[identity_store]: " __VA_ARGS__)
#define _LOGERR(...) LOG_ERROR("[identity_store]: " __VA_ARGS__)

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

char *directory_structure[] = {
    VFS_DEFAULT_NVM(0) "/config",
    LORAMAC_KEY_STORAGE_PATH,
    VFS_DEFAULT_NVM(0) "/identities",
    IDENTITY_STORAGE_PATH "self",
    IDENTITY_STORAGE_PATH "valid",
    IDENTITY_STORAGE_PATH "revoked"
};

int _write_file(char path[], const uint8_t *data, size_t data_length) {
    int fd = vfs_open(path, O_RDWR | O_CREAT, 00777);
    if(fd < 0) {
        _LOGERR("creating file %s failed with result: %d [ERROR]\n", path, fd);
        return fd;
    }
    else {
        _LOGDBG("creating file %s [OK]\n", path);
    }

    int res = vfs_write(fd, data, data_length);
    if(res < 0) {
        _LOGERR("writing to file %s failed with result: %d [ERROR]\n", path, fd);
        vfs_close(fd);
        return res;
    }
    else {
        _LOGDBG("written %d bytes to file %s [OK]\n", res, path);
    }

    vfs_close(fd);

    return 0;
}

int _read_file(char path[], uint8_t *buffer, size_t buffer_size) {
    int res = vfs_file_to_buffer(path, buffer, buffer_size);

    if (res == -ENOSPC) {
        _LOGERR("file %s larger than buffer [ERROR]\n", path);
    }
    else if (res < 0) {
        _LOGERR("error reading file %s [ERROR]\n", path);
    }

    return res;
}

int _write_root_identity_file(const identity_t *root_identity) {
    char path[] = IDENTITY_STORAGE_PATH "self/root.pubid";

    size_t out_len = MAX_IDENTITY_SIZE;
    uint8_t out_buf[out_len];
    int res = cbor_serialize_identity(root_identity, out_buf, &out_len);

    if (res < 0) {
        _LOGERR("root identity serialization failed! [ERROR]\n");
        return -1;
    }

    return _write_file(path, out_buf, out_len);
}

int _read_root_identity_file(uint8_t *buffer, size_t buffer_size) {
    char path[] = IDENTITY_STORAGE_PATH "self/root.pubid";

    return _read_file(path, buffer, buffer_size);
}

int _write_own_private_identity_file(const identity_t *private_identity) {
    char path[] = IDENTITY_STORAGE_PATH "self/self.prvid";

    size_t out_len = MAX_IDENTITY_SIZE;
    uint8_t out_buf[out_len];
    int res = cbor_serialize_identity(private_identity, out_buf, &out_len);

    if (res < 0) {
        _LOGERR("private identity serialization failed! [ERROR]\n");
        return -1;
    }

    return _write_file(path, out_buf, out_len);
}

int _read_own_private_identity_file(uint8_t *buffer, size_t buffer_size) {
    char path[] = IDENTITY_STORAGE_PATH "self/self.prvid";

    return _read_file(path, buffer, buffer_size);
}

int _write_own_public_identity_file(const signed_identity_t *signed_identity) {
    char path[] = IDENTITY_STORAGE_PATH "self/self.pubid";

    size_t out_len = MAX_IDENTITY_SIZE;
    uint8_t out_buf[out_len];
    int res = cbor_serialize_signed_public_identity(signed_identity, out_buf, &out_len);

    if (res < 0) {
        _LOGERR("signed public identity serialization failed! [ERROR]\n");
        return -1;
    }

    return _write_file(path, out_buf, out_len);
}

int _read_own_public_identity_file(uint8_t *buffer, size_t buffer_size) {
    char path[] = IDENTITY_STORAGE_PATH "self/self.pubid";

    return _read_file(path, buffer, buffer_size);
}

int _write_public_signed_identity_file(const char filename[], const signed_identity_t *signed_identity) {
    char path[sizeof(IDENTITY_STORAGE_PATH "valid/") + MAX_FILENAME_LEN] = IDENTITY_STORAGE_PATH "valid/";
    strcat(path, filename);

    size_t out_len = MAX_IDENTITY_SIZE;
    uint8_t out_buf[out_len];
    int res = cbor_serialize_signed_public_identity(signed_identity, out_buf, &out_len);

    if (res < 0) {
        _LOGERR("signed public identity serialization failed! [ERROR]\n");
        return -1;
    }

    return _write_file(path, out_buf, out_len);
}

int _read_public_signed_identity_file(const char filename[], uint8_t *buffer, size_t buffer_size) {
    char path[sizeof(IDENTITY_STORAGE_PATH "valid/") + MAX_FILENAME_LEN] = IDENTITY_STORAGE_PATH "valid/";
    strcat(path, filename);

    return _read_file(path, buffer, buffer_size);;
}

int _write_loramac_key_files(const loramac_keys_t *loramac_keys) {
    int res = _write_file(LORAMAC_KEY_STORAGE_PATH "joineui", loramac_keys->joineui, sizeof(loramac_keys->joineui));

    if (res < 0) {
        _LOGERR("_write_loramac_key_files: failed writing joineui\n");
        return -1;
    }

    res = _write_file(LORAMAC_KEY_STORAGE_PATH "deveui", loramac_keys->deveui, sizeof(loramac_keys->deveui));

    if (res < 0) {
        _LOGERR("_write_loramac_key_files: failed writing deveui\n");
        return -1;
    }

    res = _write_file(LORAMAC_KEY_STORAGE_PATH "nwkkey", loramac_keys->nwkkey, sizeof(loramac_keys->nwkkey));

    if (res < 0) {
        _LOGERR("_write_loramac_key_files: failed writing nwkkey\n");
        return -1;
    }

    return 0;
}

int get_loramac_keys(loramac_keys_t *loramac_keys_out) {
    int res = _read_file(LORAMAC_KEY_STORAGE_PATH "joineui", loramac_keys_out->joineui, sizeof(loramac_keys_out->joineui));

    if (res < 0) {
        _LOGERR("get_loramac_keys: failed getting joineui\n");
        return -1;
    }

    res = _read_file(LORAMAC_KEY_STORAGE_PATH "deveui", loramac_keys_out->deveui, sizeof(loramac_keys_out->deveui));

    if (res < 0) {
        _LOGERR("get_loramac_keys: failed getting deveui\n");
        return -1;
    }

    res = _read_file(LORAMAC_KEY_STORAGE_PATH "nwkkey", loramac_keys_out->nwkkey, sizeof(loramac_keys_out->nwkkey));

    if (res < 0) {
        _LOGERR("get_loramac_keys: failed getting nwkkey\n");
        return -1;
    }

    return 0;
}

int get_root_identity(identity_t *identity_out) {
    uint8_t buffer[MAX_IDENTITY_SIZE];
    
    int res = _read_root_identity_file(buffer, sizeof(buffer));
    if (res < 0) {
        _LOGERR("error reading root identity file [ERROR]\n");
        return -1;
    } 

    res = cbor_deserialize_identity(buffer, res, identity_out);
    if (res < 0) {
        _LOGERR("error deserializing root identity [ERROR]\n");
        return -1;
    }

    return 0;
}

int get_own_private_identity(identity_t *identity_out) {
    uint8_t buffer[MAX_IDENTITY_SIZE];
    
    int res = _read_own_private_identity_file(buffer, sizeof(buffer));
    if (res < 0) {
        _LOGERR("error reading identity file [ERROR]\n");
        return -1;
    }

    res = cbor_deserialize_identity(buffer, res, identity_out);
    if (res < 0) {
        _LOGERR("error deserializing identity [ERROR]\n");
        return -1;
    }

    return 0;
}

int get_own_signed_public_identity(signed_identity_t *signed_identity_out) {
    uint8_t buffer[MAX_IDENTITY_SIZE];

    int res = _read_own_public_identity_file(buffer, sizeof(buffer));
    if (res < 0) {
        _LOGERR("error own reading public identity file [ERROR]\n");
        return -1;
    } 

    res = cbor_deserialize_signed_identity(buffer, res, signed_identity_out);
    if (res < 0) {
        _LOGERR("error deserializing own signed identity [ERROR]\n");
        return -1;
    }

    return 0;
}

int get_own_public_identity(identity_t *identity_out) {
    signed_identity_t signed_identity;

    int res = get_own_signed_public_identity(&signed_identity);
    if (res < 0) {
        _LOGERR("error deserializing own signed public identity [ERROR]\n");
        return -1;
    }
    
    res = cbor_deserialize_identity(signed_identity.cbor_payload, sizeof(signed_identity.cbor_payload), identity_out);
    if (res < 0) {
        _LOGERR("error deserializing own public identity [ERROR]\n");
        return -1;
    }

    return 0;
}

int get_own_node_id(uint8_t *kid_buffer, size_t kid_buffer_size) {
    identity_t identity;

    int res = get_own_private_identity(&identity);

    if (res < 0) {
        _LOGERR("failed to get own node id [ERROR]");
        return -1;
    }

    // TODO: Instead of doing this, check if `kid_buffer` is large enough by comparing
    // against `kid_buffer_size` and copy the entirety of `identity.kid` into `kid_buffer`.
    memcpy(kid_buffer, identity.kid, kid_buffer_size);

    return 0;
}

int get_public_identities_init(vfs_DIR *dirp) {
    assert(dirp != NULL);

    int res = vfs_opendir(dirp, IDENTITY_STORAGE_PATH "valid");
    if (res < 0) {
        _LOGERR("failed to open %s for reading [ERROR]\n", IDENTITY_STORAGE_PATH "valid");
        return -1;
    }

    _LOGDBG("opened %s for reading\n", IDENTITY_STORAGE_PATH "valid");

    return 0;
}

int get_public_identities_next(vfs_DIR *dirp, identity_t *identity_out) {
    assert(dirp != NULL);

    static vfs_dirent_t dirent;

    int res = vfs_readdir(dirp, &dirent);

    // in case we've reached the end of the directory
    if(res == 0) {
        vfs_closedir(dirp);
        return 0;
    }

    // skip current and parent directory
    while(strcmp(dirent.d_name, ".") == 0 || strcmp(dirent.d_name, "..") == 0) {
        res = vfs_readdir(dirp, &dirent);
    }

    // in case the directory is empty
    if(res == 0) {
        vfs_closedir(dirp);
        return 0;
    }

    size_t buffer_size_out = MAX_IDENTITY_SIZE;
    uint8_t buffer[buffer_size_out];
    res = _read_public_signed_identity_file(dirent.d_name, buffer, buffer_size_out);

    if(res < 0) {
        _LOGERR("failed reading signed public identity %s\n", dirent.d_name);
        return -1;
    }

    signed_identity_t signed_identity;
    res = cbor_deserialize_signed_identity(buffer, res, &signed_identity);

    if(res < 0) {
        _LOGERR("failed deserializing signed identity [ERROR]\n");
        return -1;
    }

    res = cbor_deserialize_identity(signed_identity.cbor_payload, sizeof(signed_identity.cbor_payload), identity_out);

    if(res < 0) {
        _LOGERR("failed deserializing identity from signed public identity cbor payload [ERROR]\n");
        return -1;
    }

    return 1;
}

int add_signed_public_identity(const signed_identity_t *signed_identity) {
    identity_t identity_out;
    int res = cbor_deserialize_identity(signed_identity->cbor_payload, sizeof(signed_identity->cbor_payload), &identity_out);
    if (res < 0) {
        _LOGERR("add_signed_public_identity: error deserializing identity [ERROR]\n");
        return -1;
    }

    char filename[MAX_FILENAME_LEN];
    if(identity_out.kid[2] == DEVICE_TYPE_GATE) {
        strcpy(filename, "sensegate-");
    }
    else if(identity_out.kid[2] == DEVICE_TYPE_SENSEMATE) {
        strcpy(filename, "sensemate-");
    }
    else {
        _LOGERR("add_signed_public_identity: invalid device type [ERROR]\n");
        return -1;
    }

    char buffer[8];
    res = snprintf(buffer, sizeof(buffer), "%d", identity_out.kid[3]);
    if(res < 0) {
        _LOGERR("add_signed_public_identity: error converting id to string [ERROR]\n");
        return -1;
    }
    strcat(filename, buffer);

    res = _write_public_signed_identity_file(filename, signed_identity);
    if(res < 0) {
        _LOGERR("add_signed_public_identity: error writing public identity file [ERROR]\n");
        return -1;
    }

    return 0;
}

int _setup_flash(void) {
    int res = vfs_unmount_by_path(VFS_DEFAULT_NVM(0), false);
    _LOGINF("unmounting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    res = vfs_format_by_path(VFS_DEFAULT_NVM(0));
    _LOGINF("formatting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    res = vfs_mount_by_path(VFS_DEFAULT_NVM(0));
    _LOGINF("mounting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    for (size_t i = 0; i < ARRAY_SIZE(directory_structure); i++)
    {
        res = vfs_mkdir(directory_structure[i], 00777);
        _LOGINF("creating directory %s %s\n", directory_structure[i], ok(res == 0));
    }

    return 0;
}

int identity_store_init(void) {
    // TODO: After startup the first call to vfs_opendir will fail, so do it here.
    // Figure out why it happens.
    vfs_DIR dirp;
    int res = vfs_opendir(&dirp, "/nvm0/identities");
    if (res == 0) {
        vfs_closedir(&dirp);
    }

    for (size_t i = 0; i < ARRAY_SIZE(directory_structure); i++)
    {
        vfs_DIR dirp;
        int res = vfs_opendir(&dirp, directory_structure[i]);
        if (res == 0) {
            vfs_closedir(&dirp);
        }
        else {
            _setup_flash();
            break;
        }
    }

    if(
        !vfs_file_exists(IDENTITY_STORAGE_PATH "self/root.pubid")
        || !vfs_file_exists(IDENTITY_STORAGE_PATH "self/self.pubid")
        || !vfs_file_exists(IDENTITY_STORAGE_PATH "self/self.prvid")
        || !vfs_file_exists(LORAMAC_KEY_STORAGE_PATH "joineui")
        || !vfs_file_exists(LORAMAC_KEY_STORAGE_PATH "deveui")
        || !vfs_file_exists(LORAMAC_KEY_STORAGE_PATH "nwkkey")
    ) {
        _LOGINF("Identitiy store not set up yet, dropping to shell.\n");
        char line_buf[SHELL_DEFAULT_BUFSIZE];
        shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    }

    return 0;
}

int _provision_own_identity(int argc, char **argv) {
    if (argc > 1) {
        printf("Usage: %s\n", argv[0]);
        return -1;
    }

    _LOGINF("Run provision_keys.py to provision this node.\n");

    char buffer[512];
    char c;
    size_t i = 0;
    while(1) {
        if (i >= sizeof(buffer) + 1) {
            _LOGERR("maximum buffer size exceeded [ERROR]\n");
            return -1;
        }

        c = getchar();
        buffer[i++] = c;
        if(c == '=') {
            // we've reached the end of the base64-encoded data, null-terminate the string
            buffer[i++] = 0;
            break;
        }
    }

    _LOGDBG("identity_store_setup: received data: %s\n", buffer);

    size_t decoded_buffer_size = 2 * base64_estimate_decode_size(i);
    uint8_t decoded_buffer[decoded_buffer_size];
    _LOGDBG("identity_store_setup: decoded_buffer_size set to %d\n", decoded_buffer_size);
    int res = base64_decode(buffer, i, decoded_buffer, &decoded_buffer_size);
    if (res != BASE64_SUCCESS) {
        _LOGERR("base64 decode failed: %d [ERROR]\n", res);
    }

    if(LOG_LEVEL == LOG_DEBUG) od_hex_dump(decoded_buffer, decoded_buffer_size, 0);

    provisioning_data_t provisioning_data;
    res = cbor_deserialize_provisioning_data(decoded_buffer, decoded_buffer_size, &provisioning_data);
    if (res < 0) {
        _LOGERR("identity_store_setup: error deserializing provisioning data [ERROR]\n");
        return -1;
    }

    res = _write_root_identity_file(&provisioning_data.root_identity);
    if (res < 0) {
        _LOGERR("identity_store_setup: error writing root identity file [ERROR]\n");
        return -1;
    }

    res = _write_own_private_identity_file(&provisioning_data.private_identity);
    if (res < 0) {
        _LOGERR("identity_store_setup: error writing private identity file [ERROR]\n");
        return -1;
    }

    res = _write_own_public_identity_file(&provisioning_data.own_signed_identity);
    if (res < 0) {
        _LOGERR("identity_store_setup: error writing own public identity file [ERROR]\n");
        return -1;
    }

    res = _write_loramac_key_files(&provisioning_data.loramac_keys);
    if (res < 0) {
        _LOGERR("identity_store_setup: error writing loramac key files [ERROR]\n");
        return -1;
    }

    return 0;
}

int _delete_known_identities(int argc, char **argv)
{
    if (argc > 1) {
        printf("Usage: %s\n", argv[0]);
        return -1;
    }

    char buffer[128];
    int res = vfs_unlink_recursive(IDENTITY_STORAGE_PATH "valid", buffer, sizeof(buffer));
    if (res < 0) {
        _LOGERR("failed deleting learned public keys [ERROR]\n");
        return -1;
    }

    res = vfs_mkdir(IDENTITY_STORAGE_PATH "valid", 00777);
    if (res < 0) {
        _LOGERR("failed recreating valid keys directory [ERROR]\n");
        return -1;
    }

    _LOGINF("deleted all learned public keys from other nodes\n");
    return 0;
}

int _print_identity(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s /path/to/file\n", argv[0]);
        return -1;
    }

    uint8_t buffer[MAX_IDENTITY_SIZE];
    int res = _read_file(argv[1], buffer, sizeof(buffer));

    if(res < 0) {
        _LOGERR("failed reading file %s", argv[1]);
        return -1;
    }

    for (int i = 0; i < res; i++) {
        printf("%02X", buffer[i]);
    }
    printf("\n");

    return 0;
}

SHELL_COMMAND(delete_learned_identities, "Delete learned identities from external flash", _delete_known_identities);
SHELL_COMMAND(provision_own_identity, "Provision own identity", _provision_own_identity);
SHELL_COMMAND(print_identity, "Print identity as hex-encoded bytestring", _print_identity);
