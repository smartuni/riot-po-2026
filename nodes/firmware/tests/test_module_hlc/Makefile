APPLICATION = test_module_hlc
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

# This application provides a physical clock to the HLC module using RTC
USEMODULE += periph_rtc
USEPKG += unity
USEMODULE += hybrid_logical_clock

include $(RIOTBASE)/Makefile.include
