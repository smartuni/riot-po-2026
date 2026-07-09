#include <stdio.h>
#include <fcntl.h>
#include <cbor.h>

#include "vfs_util.h"
#include "identity_store.h"
#include "ed25519_key_entry.h"
#include "base64.h"
#include "cose.h"
#include "od.h"

#define LOG_LEVEL LOG_DEBUG
#include "log.h"

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

int write_file(char path[], const uint8_t *data, size_t data_length) {
    int fd = vfs_open(path, O_RDWR | O_CREAT, 00777);
    if(fd < 0) {
        LOG_ERROR("creating file %s failed with result: %d [FAILED]\n", path, fd);
        return fd;
    }
    else {
        LOG_DEBUG("creating file %s [OK]\n", path);
    }

    int res = vfs_write(fd, data, data_length);
    if(res < 0) {
        LOG_ERROR("writing to file %s failed with result: %d [FAILED]\n", path, fd);
        vfs_close(fd);
        return res;
    }
    else {
        LOG_DEBUG("written %d bytes to file %s [OK]\n", res, path);
    }

    vfs_close(fd);

    return 0;
}

int read_file(char path[], uint8_t *buffer, size_t buffer_size) {
    int fd = vfs_open(path, O_RDONLY, 00777);
    if(fd < 0) {
        LOG_ERROR("opening file %s failed with result: %d [FAILED]\n", path, fd);
        return fd;
    }
    else {
        LOG_DEBUG("opening file %s [OK]\n", path);
    }

    uint8_t c;
    size_t i = 0;
    while (read(fd, &c, 1) != 0) { // replace with vfs_file_to_buffer
        if(i >= buffer_size) {
            LOG_ERROR("identity file %s larger than buffer [FAILED]\n", path);
            vfs_close(fd);
            return -1;
        }
        buffer[i] = c;
        i++;
    }
    LOG_DEBUG("identity file %s copied to buffer\n", path);

    vfs_close(fd);

    return 0;
}

int write_root_identity(const uint8_t *kid, size_t kid_len, const uint8_t *key, size_t key_len) {
    char path[] = IDENTITY_STORAGE_PATH "self/root.pubid";

    uint8_t buffer[2 + kid_len + 2 + key_len];
    CborEncoder encoder;
    cbor_encoder_init(&encoder, buffer, sizeof(buffer), 0);
    CborEncoder array_encoder;
    cbor_encoder_create_array(&encoder, &array_encoder, 2);
    cbor_encode_byte_string(&array_encoder, kid, kid_len);
    cbor_encode_byte_string(&array_encoder, key, key_len);
    cbor_encoder_close_container(&encoder, &array_encoder);

    return write_file(path, buffer, sizeof(buffer));
}

int read_root_identity(uint8_t *buffer, size_t buffer_size) {
    char path[] = IDENTITY_STORAGE_PATH "self/root.pubid";

    return read_file(path, buffer, buffer_size);
}

int write_private_identity(const uint8_t *kid, size_t kid_len, const uint8_t *key, size_t key_len) {
    char path[] = IDENTITY_STORAGE_PATH "self/self.prvid";

    uint8_t buffer[2 + kid_len + 2 + key_len];
    CborEncoder encoder;
    cbor_encoder_init(&encoder, buffer, sizeof(buffer), 0);
    CborEncoder array_encoder;
    cbor_encoder_create_array(&encoder, &array_encoder, 2);
    cbor_encode_byte_string(&array_encoder, kid, kid_len);
    cbor_encode_byte_string(&array_encoder, key, key_len);
    cbor_encoder_close_container(&encoder, &array_encoder);

    return write_file(path, buffer, sizeof(buffer));
}

int read_private_identity(uint8_t *buffer, size_t buffer_size) {
    char path[] = IDENTITY_STORAGE_PATH "self/self.prvid";

    return read_file(path, buffer, buffer_size);
}

int write_public_identity(const uint8_t *signed_id, size_t signed_id_len, const uint8_t *signature, size_t signature_len) {
    char path[] = IDENTITY_STORAGE_PATH "self/self.pubid";

    od_hex_dump(signed_id, signed_id_len, 0);
    od_hex_dump(signature, signature_len, 0);

    uint8_t buffer[256];
    CborEncoder encoder;
    cbor_encoder_init(&encoder, buffer, sizeof(buffer), 0);
    CborEncoder array_encoder;
    cbor_encoder_create_array(&encoder, &array_encoder, 2);
    cbor_encode_byte_string(&array_encoder, signed_id, signed_id_len);
    cbor_encode_byte_string(&array_encoder, signature, signature_len);
    cbor_encoder_close_container(&encoder, &array_encoder);
    size_t buffer_size = cbor_encoder_get_buffer_size(&encoder, buffer);

    od_hex_dump(buffer, buffer_size, 0);

    return write_file(path, buffer, buffer_size);
}

int read_public_identity(uint8_t *buffer, size_t buffer_size) {
    char path[] = IDENTITY_STORAGE_PATH "self/self.pubid";

    return read_file(path, buffer, buffer_size);
}

int write_identity(char filename[], const uint8_t *data, size_t data_length) {
    char path[sizeof(IDENTITY_STORAGE_PATH) + 6 + MAX_KID_LEN] = IDENTITY_STORAGE_PATH "valid/";
    strcat(path, filename);

    return write_file(path, data, data_length);
}

int read_identity(char filename[], uint8_t *buffer, size_t buffer_size) {
    char path[sizeof(IDENTITY_STORAGE_PATH) + 6 + MAX_KID_LEN] = IDENTITY_STORAGE_PATH "valid/";
    strcat(path, filename);

    return read_file(path, buffer, buffer_size);;
}

int get_root_identity(identity_t *identity) {
    uint8_t buffer[MAX_IDENTITY_SIZE];
    read_root_identity(buffer, sizeof(buffer));
    
    size_t length = 0;
    CborParser parser;
    CborValue it;
    CborError err = cbor_parser_init(buffer, sizeof(buffer), 0, &parser, &it);
    if(err) {
        printf("%s\n", cbor_error_string(err));
    }
    CborType type = cbor_value_get_type(&it);
    if(type != CborArrayType) {
        printf("No outer array found\n");
    }
    cbor_value_get_array_length(&it, &length);
    if(length != 2) {
        printf("Unexpected number of array items\n");
    }
    cbor_value_enter_container(&it, &it);

    type = cbor_value_get_type(&it);
    if(type != CborByteStringType) {
        printf("Not a byte string\n");
    }
    cbor_value_get_string_length(&it, &length);
    cbor_value_copy_byte_string(&it, identity->kid, &length, &it);

    // root pubkey
    type = cbor_value_get_type(&it);
    if(type != CborByteStringType) {
        printf("Not a byte string\n");
    }
    cbor_value_get_string_length(&it, &length);
    cbor_value_copy_byte_string(&it, identity->key, &length, &it);

    return 0;
}

int get_private_identity(identity_t *identity) {
    uint8_t buffer[MAX_IDENTITY_SIZE];
    read_private_identity(buffer, sizeof(buffer));

    size_t length = 0;
    CborParser parser;
    CborValue it;
    CborError err = cbor_parser_init(buffer, sizeof(buffer), 0, &parser, &it);
    if(err) {
        printf("%s\n", cbor_error_string(err));
    }
    CborType type = cbor_value_get_type(&it);
    if(type != CborArrayType) {
        printf("No outer array found\n");
    }
    cbor_value_get_array_length(&it, &length);
    if(length != 2) {
        printf("Unexpected number of array items\n");
    }
    cbor_value_enter_container(&it, &it);

    type = cbor_value_get_type(&it);
    if(type != CborByteStringType) {
        printf("Not a byte string\n");
    }
    cbor_value_get_string_length(&it, &length);
    cbor_value_copy_byte_string(&it, identity->kid, &length, &it);

    type = cbor_value_get_type(&it);
    if(type != CborByteStringType) {
        printf("Not a byte string\n");
    }
    cbor_value_get_string_length(&it, &length);
    cbor_value_copy_byte_string(&it, identity->key, &length, &it);

    return 0;
}

int get_public_identity(identity_t *identity) {
    uint8_t buffer[MAX_IDENTITY_SIZE];
    uint8_t inner_buffer[MAX_IDENTITY_SIZE];
    read_public_identity(buffer, sizeof(buffer));
    
    size_t length = 0;
    CborParser parser;
    CborValue it;
    CborError err = cbor_parser_init(buffer, sizeof(buffer), 0, &parser, &it);
    if(err) {
        printf("%s\n", cbor_error_string(err));
    }
    CborType type = cbor_value_get_type(&it);
    if(type != CborArrayType) {
        printf("No outer array found\n");
    }
    cbor_value_get_array_length(&it, &length);
    if(length != 2) {
        printf("Unexpected number of array items\n");
    }
    cbor_value_enter_container(&it, &it);

    type = cbor_value_get_type(&it);
    if(type != CborByteStringType) {
        printf("Not a byte string\n");
    }
    cbor_value_get_string_length(&it, &length);
    cbor_value_copy_byte_string(&it, inner_buffer, &length, &it);

    

    length = 0;
    CborParser inner_parser;
    CborValue inner_it;
    err = cbor_parser_init(inner_buffer, sizeof(inner_buffer), 0, &inner_parser, &inner_it);
    if(err) {
        printf("%s\n", cbor_error_string(err));
    }
    type = cbor_value_get_type(&inner_it);
    if(type != CborArrayType) {
        printf("No outer array found\n");
    }
    cbor_value_get_array_length(&inner_it, &length);
    if(length != 2) {
        printf("Unexpected number of array items\n");
    }
    cbor_value_enter_container(&inner_it, &inner_it);

    type = cbor_value_get_type(&inner_it);
    if(type != CborByteStringType) {
        printf("Not a byte string\n");
    }
    cbor_value_get_string_length(&inner_it, &length);
    cbor_value_copy_byte_string(&inner_it, identity->kid, &length, &inner_it);

    type = cbor_value_get_type(&inner_it);
    if(type != CborByteStringType) {
        printf("Not a byte string\n");
    }
    cbor_value_get_string_length(&inner_it, &length);
    cbor_value_copy_byte_string(&inner_it, identity->key, &length, &inner_it);


    return 0;
}

int get_self_node_id(uint8_t *kid_buffer, size_t kid_buffer_size) {
    identity_t identity;
    get_private_identity(&identity);
    memcpy(kid_buffer, identity.kid, kid_buffer_size);
    return 0;
}

int get_self_signed_pubid(signed_identity_t *signed_identity) {
    uint8_t buffer[MAX_IDENTITY_SIZE];
    read_public_identity(buffer, sizeof(buffer));
    
    size_t length = 0;
    CborParser parser;
    CborValue it;
    CborError err = cbor_parser_init(buffer, sizeof(buffer), 0, &parser, &it);
    if(err) {
        printf("%s\n", cbor_error_string(err));
    }
    CborType type = cbor_value_get_type(&it);
    if(type != CborArrayType) {
        printf("No outer array found\n");
    }
    cbor_value_get_array_length(&it, &length);
    if(length != 2) {
        printf("Unexpected number of array items\n");
    }
    cbor_value_enter_container(&it, &it);

        type = cbor_value_get_type(&it);
    if(type != CborByteStringType) {
        printf("Not a byte string\n");
    }
    cbor_value_get_string_length(&it, &length);
    cbor_value_copy_byte_string(&it, signed_identity->cbor_payload, &length, &it);

    type = cbor_value_get_type(&it);
    if(type != CborByteStringType) {
        printf("Not a byte string\n");
    }
    cbor_value_get_string_length(&it, &length);
    cbor_value_copy_byte_string(&it, signed_identity->signature, &length, &it);

    return 0;
}

/*
int provide_public_keys(void) {
    vfs_DIR dirp;
    int res = vfs_opendir(&dirp, IDENTITY_STORAGE_PATH "/valid");
    //_LOGDBG("opendir result %d\n", res);
    if(res == 0) {
        vfs_dirent_t dirent;
        while((res = vfs_readdir(&dirp, &dirent)) == 1) {
            if(strcmp(dirent.d_name, ".") != 0 && strcmp(dirent.d_name, "..") != 0) {
                puts(dirent.d_name);

                
                uint8_t identity[MAX_IDENTITY_SIZE];
                uint8_t ed25519_public_key[32];
                uint8_t kid[4];


                int res = read_identity(dirent.d_name, identity, MAX_IDENTITY_SIZE);
                //res = verify_identity();
                res = parse_identity(identity, MAX_IDENTITY_SIZE, ed25519_public_key, 32, kid, sizeof(kid));
                
                for (size_t i = 0; i < 32; i++) {
                    printf("0x%02x ", ed25519_public_key[i]);
                }
                printf("\n");

                
                int err = credential_manager_add_key(kid, sizeof(kid), 0, ed25519_public_key, sizeof(ed25519_public_key));
                if (err) {
                    _LOGERR("add public key [FAILED]");
                    return -5;
                }
                
            }
        }
    }
    vfs_closedir(&dirp);
    return 0;
}
*/

char *directory_structure[] = {
    IDENTITY_STORAGE_PATH,
    IDENTITY_STORAGE_PATH "self",
    IDENTITY_STORAGE_PATH "valid",
    IDENTITY_STORAGE_PATH "revoked"
};

int setup_flash(void) {
    int res = vfs_unmount_by_path(VFS_DEFAULT_NVM(0), false);
    LOG_INFO("unmounting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    res = vfs_format_by_path(VFS_DEFAULT_NVM(0));
    LOG_INFO("formatting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    res = vfs_mount_by_path(VFS_DEFAULT_NVM(0));
    LOG_INFO("mounting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    for (size_t i = 0; i < ARRAY_SIZE(directory_structure); i++)
    {
        res = vfs_mkdir(directory_structure[i], 00777);
        LOG_INFO("creating directory %s %s\n", directory_structure[i], ok(res == 0));
    }

    return 0;
}

int identity_store_setup(void) {
    for (size_t i = 0; i < ARRAY_SIZE(directory_structure); i++)
    {
        vfs_DIR dirp;
        int res = vfs_opendir(&dirp, directory_structure[i]);
        if(res == 0) {
            vfs_closedir(&dirp);
        }
        else {
            setup_flash();
            break;
        }
    }

    if(
        !vfs_file_exists(IDENTITY_STORAGE_PATH "self/root.pubid")
        || !vfs_file_exists(IDENTITY_STORAGE_PATH "self/self.pubid")
        || !vfs_file_exists(IDENTITY_STORAGE_PATH "self/self.prvid")
    ) {
        char buffer[512];
        fgets(buffer, 512, stdin);
        size_t decoded_buffer_size = 2 * base64_estimate_decode_size(strlen(buffer));
        uint8_t decoded_buffer[decoded_buffer_size];
        int res = base64_decode(buffer, sizeof(buffer), decoded_buffer, &decoded_buffer_size);
        if(res != BASE64_SUCCESS) {
            printf("base64 decode failed: %d\n", res);
        }
        od_hex_dump(decoded_buffer, decoded_buffer_size, 0);

        size_t length = 0;
        CborParser parser;
        CborValue it;
        CborError err = cbor_parser_init(decoded_buffer, decoded_buffer_size, 0, &parser, &it);
        if(err) {
            printf("%s\n", cbor_error_string(err));
        }
        CborType type = cbor_value_get_type(&it);
        if(type != CborArrayType) {
            printf("No outer array found\n");
        }
        cbor_value_get_array_length(&it, &length);
        if(length != 6) {
            printf("Unexpected number of array items\n");
        }
        cbor_value_enter_container(&it, &it);

        // root kid
        type = cbor_value_get_type(&it);
        if(type != CborByteStringType) {
            printf("Not a byte string\n");
        }
        cbor_value_get_string_length(&it, &length);
        uint8_t root_kid[KID_LEN];
        cbor_value_copy_byte_string(&it, root_kid, &length, &it);

        // root pubkey
        type = cbor_value_get_type(&it);
        if(type != CborByteStringType) {
            printf("Not a byte string\n");
        }
        cbor_value_get_string_length(&it, &length);
        uint8_t root_pubkey[ED25519_KEY_LEN];
        cbor_value_copy_byte_string(&it, root_pubkey, &length, &it);

        write_root_identity(root_kid, sizeof(root_kid), root_pubkey, sizeof(root_pubkey));

        // self kid
        type = cbor_value_get_type(&it);
        if(type != CborByteStringType) {
            printf("Not a byte string\n");
        }
        cbor_value_get_string_length(&it, &length);
        uint8_t self_kid[KID_LEN];
        cbor_value_copy_byte_string(&it, self_kid, &length, &it);

        // self privkey
        type = cbor_value_get_type(&it);
        if(type != CborByteStringType) {
            printf("Not a byte string\n");
        }
        cbor_value_get_string_length(&it, &length);
        uint8_t self_privkey[ED25519_KEY_LEN];
        cbor_value_copy_byte_string(&it, self_privkey, &length, &it);

        write_private_identity(self_kid, sizeof(self_kid), self_privkey, sizeof(self_privkey));

        // self pubid
        type = cbor_value_get_type(&it);
        if(type != CborByteStringType) {
            printf("Not a byte string\n");
        }
        cbor_value_get_string_length(&it, &length);
        uint8_t self_pubid[PUBID_LEN];
        cbor_value_copy_byte_string(&it, self_pubid, &length, &it);

        // self pubid signature
        type = cbor_value_get_type(&it);
        if(type != CborByteStringType) {
            printf("Not a byte string\n");
        }
        cbor_value_get_string_length(&it, &length);
        uint8_t self_pubid_signature[PUBID_SIGNATURE_LEN];
        cbor_value_copy_byte_string(&it, self_pubid_signature, &length, &it);

        write_public_identity(self_pubid, sizeof(self_pubid), self_pubid_signature, sizeof(self_pubid_signature));
    }

    return 0;
}
