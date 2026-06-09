#pragma once

#include "vfs.h"
#include "vfs_default.h"

#define CERTIFICATE_STORAGE_PATH "/nvm0/cred/public_certs/"
#define MAX_CERTIFICATE_SIZE 1024

int write_private_key(const void *data, size_t data_length);

int write_certificate(char filename[], const void *data, size_t data_length);

int read_certificate(char filename[], const void *buffer, size_t buffer_size);
