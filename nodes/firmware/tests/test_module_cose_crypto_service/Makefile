APPLICATION = test_module_cose_crypto_service
BOARD = native
QUIET = 1
DEVELHELP = 1

# Paths
PROJECT_BASE = $(CURDIR)/../..
RIOTBASE = $(PROJECT_BASE)/RIOT
EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules
EXTERNAL_PKG_DIRS += $(PROJECT_BASE)/custom_packages

# terminate native when the test is complete
CFLAGS += -DNATIVE_AUTO_EXIT=1

USEPKG += unity
USEMODULE += cose_crypto_service

USEPKG += unity
USEMODULE += flashdb_store_service
# select the VFS backend
USEMODULE += flashdb_vfs
USEMODULE += flashdb_kvdb

USEMODULE += vfs_default
USEMODULE += vfs_auto_format

include $(RIOTBASE)/Makefile.include
