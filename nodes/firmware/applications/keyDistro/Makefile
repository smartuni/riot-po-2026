# name of your application
APPLICATION = KeyDistro

# If no BOARD is found in the environment, use this default:
BOARD ?= adafruit-feather-nrf52840-sense

PROJECT_BASE ?= $(CURDIR)/../..
RIOTBASE ?= $(PROJECT_BASE)/RIOT
EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules

RIOT_CONFIG_DEVICE_ID ?= 20
RIOT_CONFIG_DEVICE_TYPE = DEVICE_TYPE_GATE

# Enable the milliseconds timer.
USEMODULE += ztimer
USEMODULE += ztimer_sec

USEMODULE += vfs_default
USEMODULE += mtd

USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

USEMODULE += key-distro
USEMODULE += certificate_store

CFLAGS += -DRIOT_CONFIG_DEVICE_ID=$(RIOT_CONFIG_DEVICE_ID)
CFLAGS += -DRIOT_CONFIG_DEVICE_TYPE=$(RIOT_CONFIG_DEVICE_TYPE)

# For LittleFS on real devices, the main stack size has to be
# increased:
CFLAGS += -DTHREAD_STACKSIZE_MAIN=4096

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Some workarounds are needed in order to get the tutorial running on
# some computers.
PREFLASH_DELAY=8

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1
CFLAGS += -DDEBUG_ASSERT_VERBOSE
include $(RIOTBASE)/Makefile.include
