APPLICATION = test_module_cbor_serialization
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
USEMODULE += od
USEMODULE += tables
USEMODULE += cbor_serialization

include $(RIOTBASE)/Makefile.include
