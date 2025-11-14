APPLICATION = test_module_flashdb_store_service
BOARD = native
QUIET = 1
DEVELHELP = 1

# Paths
PROJECT_BASE = $(CURDIR)/../..
RIOTBASE = $(PROJECT_BASE)/RIOT
# select the VFS backend
USEMODULE += flashdb_vfs
USEMODULE += flashdb_kvdb

USEMODULE += vfs_default
USEMODULE += vfs_auto_format

CFLAGS += -DTHREAD_STACKSIZE_MAIN=THREAD_STACKSIZE_LARGE
EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules
EXTERNAL_PKG_DIRS += $(PROJECT_BASE)/custom_packages

# terminate native when the test is complete
CFLAGS += -DNATIVE_AUTO_EXIT=1

USEPKG += unity

USEMODULE += od
USEMODULE += flashdb_store_service

include $(RIOTBASE)/Makefile.include
