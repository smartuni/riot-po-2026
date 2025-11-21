APPLICATION = test_module_tables
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

# include internal headers to test internal functionalities
CFLAGS += -I$(EXTERNAL_MODULE_DIRS)/tables

USEPKG += unity

USEMODULE += tables
USEMODULE += crypto_service
USEMODULE += od
USEMODULE += hlc_ztimer

include ../ci.mk
include $(RIOTBASE)/Makefile.include
