APPLICATION = test_pkg_embeddb
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

USEPKG += embeddb
USEPKG += unity

# Use littleFS as virtual FS backend
USEMODULE += littlefs

# Use an emulated RAM-based MTD for test as memory device for the FS
USEMODULE += mtd_emulated

include $(RIOTBASE)/Makefile.include
