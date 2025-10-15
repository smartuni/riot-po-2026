APPLICATION = test_tables_module
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

# Needed application configurations
CFLAGS += -DRIOT_CONFIG_DEVICE_ID=1
CFLAGS += -DRIOT_CONFIG_DEVICE_TYPE=0

USEPKG += unity

# TODO: need to disable?
DISABLE_MODULE += auto_init auto_init_%

USEMODULE += tables

include $(RIOTBASE)/Makefile.include
