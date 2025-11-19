APPLICATION = test_module_flashdb_store_service
BOARD = native
QUIET = 1
DEVELHELP = 1

# Paths
PROJECT_BASE = $(CURDIR)/../..
RIOTBASE = $(PROJECT_BASE)/RIOT
# select the VFS backend
USEMODULE += flashdb_vfs

# sleep before the test to wait for stdio
USEMODULE += ztimer_msec
CFLAGS += -DTHREAD_STACKSIZE_MAIN=THREAD_STACKSIZE_LARGE*2

VERBOSE_ASSERT ?= 1

# use emulated mtd (virtual device in RAM) to allow testing on all platforms
USEMODULE += mtd_emulated

# Size configuration for the emulated mtd device
CFLAGS+=-DSECTOR_COUNT=16
CFLAGS+=-DPAGE_PER_SECTOR=8
CFLAGS+=-DPAGE_SIZE=64

# use half of the filesystem for flashdb
CFLAGS+=-DFLASHDB_STORE_SERVICE_SECTOR_SIZE=\(PAGE_PER_SECTOR*PAGE_SIZE\)
CFLAGS+=-DFLASHDB_STORE_SERVICE_DB_SIZE=\(\(FLASHDB_STORE_SERVICE_SECTOR_SIZE*SECTOR_COUNT\)/2\)

USEPKG += littlefs2

EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules
EXTERNAL_PKG_DIRS += $(PROJECT_BASE)/custom_packages

# terminate native when the test is complete
CFLAGS += -DNATIVE_AUTO_EXIT=1

USEPKG += unity

USEMODULE += od
USEMODULE += flashdb_store_service
include $(RIOTBASE)/Makefile.include
