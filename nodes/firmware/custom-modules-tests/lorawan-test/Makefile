# This has to be the absolute path to the RIOT base directory:
APPLICATION = lorawan_test
PROJECT_BASE ?= $(CURDIR)/../..
RIOTBASE ?= $(PROJECT_BASE)/RIOT

EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules

# If no BOARD is found in the environment, use this default:
BOARD ?= feather-nrf52840-sense

# Eigene Module aktivieren
#USEMODULE += cose-service

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Some workarounds are needed in order to get the tutorial running on
# some computers.
-include ../../lab_workarounds.mk

USEMODULE += mate_lorawan
EXTERNAL_MODULE_DIRS += ../../mate_lorawan

CFLAGS += -DTHREAD_STACKSIZE_MAIN=4096


# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

CFLAGS += -DDEVICE_ID=$(RIOT_CONFIG_DEVICE_ID) #Device ID- change when flashing
CFLAGS += -DDEVICE_TYPE=3 #Device Type

include $(RIOTBASE)/Makefile.include
