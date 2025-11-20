APPLICATION = test_module_hlc
BOARD ?= adafruit-feather-nrf52840-sense
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
USEMODULE += hybrid_logical_clock
USEMODULE += hlc_ztimer

include ../ci.mk
include $(RIOTBASE)/Makefile.include
