#include <stdio.h>
#include <fcntl.h>

#include "ztimer.h"
#include "shell.h"
#include "vfs.h"
#include "vfs_default.h"
#include "key_config.h"
#include "secrets/public_keys.h"

#define LOG_LEVEL LOG_DEBUG
#include "log.h"

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

int write_key(char path[], const void *key_data, size_t key_length)
{
    int fd = vfs_open(path, O_RDWR | O_CREAT, 00777);
    if(fd < 0) {
        LOG_ERROR("creating file %s failed with result: %d [FAILED]\n", path, fd);
        return fd;
    }
    else {
        LOG_DEBUG("creating file %s [OK]\n", path);
    }

    int res = vfs_write(fd, key_data, key_length);
    if(res < 0) {
        LOG_ERROR("writing to file %s failed with result: %d [FAILED]\n", path, fd);
        return res;
    }
    else {
        LOG_DEBUG("written %d bytes to file %s [OK]\n", res, path);
    }

    vfs_close(fd);

    return 0;
}

static int _print_key(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int fd = vfs_open(argv[1], O_RDONLY, 00777);
    if(fd < 0) return -1;

    uint8_t c;
    while (read(fd, &c, 1) != 0) {
        printf("0x%02x ", c);
    }
    printf("\n");

    vfs_close(fd);

    return 0;
}

SHELL_COMMAND(print_key, "Print a key", _print_key);

int main(void)
{
    ztimer_sleep(ZTIMER_SEC, 10);
    puts("[main]: starting");

    (void)ed25519_public_key;

    int res = vfs_unmount_by_path(VFS_DEFAULT_NVM(0), false);
    LOG_INFO("unmounting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    res = vfs_format_by_path(VFS_DEFAULT_NVM(0));
    LOG_INFO("formatting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    res = vfs_mount_by_path(VFS_DEFAULT_NVM(0));
    LOG_INFO("mounting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    res = vfs_mkdir(VFS_DEFAULT_NVM(0) "/cred", 00777);
    LOG_INFO("creating directory %s %s\n", VFS_DEFAULT_NVM(0) "/cred", ok(res == 0));

    res = vfs_mkdir(VFS_DEFAULT_NVM(0) "/cred/public_keys", 00777);
    LOG_INFO("creating directory %s %s\n", VFS_DEFAULT_NVM(0) "/cred/public_keys", ok(res == 0));

    // write private key
    res = write_key(VFS_DEFAULT_NVM(0) "/cred/private_key", &ed25519_secret_key, sizeof(ed25519_secret_key));
    LOG_INFO("writing private key %s\n", ok(res == 0));

    // write public keys
    for (size_t i = 0; i < ARRAY_SIZE(known_keys); i++)
    {
        char path[] = VFS_DEFAULT_NVM(0) "/cred/public_keys/";
        strcat(path, (char*) known_keys[i].kid);
        res = write_key(path, &known_keys[i].public_key, sizeof(known_keys[i].public_key));
        LOG_INFO("writing public key %s %s\n", (char*) known_keys[i].kid, ok(res == 0));
    }

    // drop to shell for inspecting keys and debugging
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
