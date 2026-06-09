#include <stdio.h>
#include <fcntl.h>

#include "certificate_store.h"
#include "ed25519_key_entry.h"

#define LOG_LEVEL LOG_DEBUG
#include "log.h"

int write_file(char path[], const void *data, size_t data_length) {
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

int write_private_key(const void *data, size_t data_length) {
    char path[] = "/nvm0/cred/private_key";

    return write_file(path, data, data_length);
}

int write_certificate(char filename[], const void *data, size_t data_length) {
    char path[sizeof(CERTIFICATE_STORAGE_PATH) + MAX_KID_LEN] = CERTIFICATE_STORAGE_PATH;
    strcat(path, filename);

    return write_file(path, data, data_length);
};

int read_certificate(char filename[], const void *buffer, size_t buffer_size) {
    char path[sizeof(CERTIFICATE_STORAGE_PATH) + MAX_KID_LEN] = CERTIFICATE_STORAGE_PATH;
    strcat(path, filename);

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
    while (read(fd, &c, 1) != 0) {
        if(i >= buffer_size) {
            LOG_ERROR("certificate %s larger than buffer [FAILED]\n", path);
            vfs_close(fd);
            return -1;
        }
        ((uint8_t*) buffer)[i] = c;
        i++;
    }
    LOG_DEBUG("certificate %s copied to buffer\n", path);

    vfs_close(fd);

    return 0;
};
