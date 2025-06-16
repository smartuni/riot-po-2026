# name of your application
APPLICATION = tables-test

# If no BOARD is found in the environment, use this default:
BOARD ?= feather-nrf52840-sense

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT
PROJECT_BASE ?= $(CURDIR)/../..
EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Some workarounds are needed in order to get the tutorial running on
# some computers.
-include ../../lab_workarounds.mk

# This board requires a start sleep to actually catch the printed output
USEMODULE += ztimer_sec


USEMODULE += tables

# Include other packages here e.g.
# include ../../other_package/other_package.inc.mk

# We use shell in this example
USEMODULE += shell

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

CFLAGS += -DDEBUG_ASSERT_VERBOSE

include $(RIOTBASE)/Makefile.include
