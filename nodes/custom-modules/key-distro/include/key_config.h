#ifndef KEY_CONFIG_H
#define KEY_CONFIG_H

#ifdef TEST_KEYS
    #include "test_private_key.h"
    #include "test_public_keys.h"
#else
    #define _STR(x) #x
	#define STR(x) _STR(x)

	#define FILE_PATH(dev) STR(secrets/dev##_private_key.h)
	#define INCLUDE_FILE(dev) FILE_PATH(dev)

	#include INCLUDE_FILE(DEVICE_ID)
	#include "secrets/public_keys.h"
#endif

#endif // KEY_CONFIG_H
