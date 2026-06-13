#include <stdio.h>
#include <fcntl.h>

#include "ztimer.h"
#include "shell.h"
#include "vfs.h"
#include "vfs_default.h"
#include "key_config.h"
#include "secrets/public_keys.h"
#include "certificate_store.h"

#define LOG_LEVEL LOG_DEBUG
#include "log.h"

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

static int _print_key(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    uint8_t buffer[MAX_CERTIFICATE_SIZE];
    int res = read_certificate(argv[1], buffer, MAX_CERTIFICATE_SIZE);
    if(res != 0) return res;

    for (size_t i = 0; i < MAX_CERTIFICATE_SIZE; i++)
    {
        if(buffer[i] == 0) break;
        printf("0x%02x ", buffer[i]);
    }
    printf("\n");

    return 0;
}

SHELL_COMMAND(print_key, "Print a key", _print_key);

int main(void)
{
    ztimer_sleep(ZTIMER_SEC, 10);
    puts("[main]: starting");

    (void)ed25519_public_key;
    //(void)ed25519_secret_key;

    //decode_c509_certificate();

    int res = vfs_unmount_by_path(VFS_DEFAULT_NVM(0), false);
    LOG_INFO("unmounting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    res = vfs_format_by_path(VFS_DEFAULT_NVM(0));
    LOG_INFO("formatting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    res = vfs_mount_by_path(VFS_DEFAULT_NVM(0));
    LOG_INFO("mounting %s %s\n", VFS_DEFAULT_NVM(0), ok(res == 0));

    res = vfs_mkdir(VFS_DEFAULT_NVM(0) "/cred", 00777);
    LOG_INFO("creating directory %s %s\n", VFS_DEFAULT_NVM(0) "/cred", ok(res == 0));

    res = vfs_mkdir(VFS_DEFAULT_NVM(0) "/cred/public_certs", 00777);
    LOG_INFO("creating directory %s %s\n", VFS_DEFAULT_NVM(0) "/cred" "/public_certs", ok(res == 0));

    // write private key
    res = write_private_key(ed25519_secret_key, sizeof(ed25519_secret_key));
    LOG_INFO("writing private key %s\n", ok(res == 0));

    // write public keys
    for (size_t i = 0; i < ARRAY_SIZE(known_keys); i++)
    {
        char filename[known_keys[i].kid_len + 1];
        memcpy(filename, known_keys[i].kid, known_keys[i].kid_len);
        filename[known_keys[i].kid_len] = 0;
        res = write_certificate(filename, known_keys[i].public_key, sizeof(known_keys[i].public_key));
        LOG_INFO("writing certificate %s %s\n", filename, ok(res == 0));
    }


    // drop to shell for inspecting keys and debugging
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
