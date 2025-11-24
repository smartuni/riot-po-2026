# name of your application
APPLICATION = sensegate

# If no BOARD is found in the environment, use this default:
BOARD ?= adafruit-feather-nrf52840-sense

PROJECT_BASE ?= $(CURDIR)/../..
RIOTBASE ?= $(PROJECT_BASE)/RIOT
EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules

# Configrue device id and device type.
# This will instruct the key-distro module to include required credentials.
RIOT_CONFIG_DEVICE_ID ?= 1
RIOT_CONFIG_DEVICE_TYPE ?= GATE_NODE

USEMODULE += periph_gpio
USEMODULE += periph_gpio_irq

# Enable the milliseconds timer.
USEMODULE += ztimer
USEMODULE += ztimer_msec
USEMODULE += ztimer_sec


USEPKG += u8g2
USEMODULE += event
USEMODULE += event_thread_highest
USEMODULE += event_thread
USEMODULE += event_timeout
USEMODULE += tables_old
USEMODULE += mate_lorawan
USEMODULE += mate_ble
USEMODULE += inductive_sensor

CFLAGS += -DRIOT_CONFIG_DEVICE_ID=$(RIOT_CONFIG_DEVICE_ID)
CFLAGS += -DRIOT_CONFIG_DEVICE_TYPE=$(RIOT_CONFIG_DEVICE_TYPE)
CFLAGS += -DEVENT_THREAD_HIGHEST_STACKSIZE=3*1024
CFLAGS += -DEVENT_THREAD_MEDIUM_STACKSIZE=3*1024

CFLAGS += -DTHREAD_STACKSIZE_MAIN=THREAD_STACKSIZE_LARGE*2

# RAM-based storage backend
USEMODULE += mtd_emulated

# credential manager stuff
USEMODULE += credential_manager
USEMODULE += flashdb_store_service
USEMODULE += vfs_default
USEMODULE += vfs_auto_format

# tables stuff
USEMODULE += tables
USEMODULE += cose_crypto_service
USEMODULE += hlc_ztimer

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
CFLAGS += -I$(APPDIR)/include
include $(RIOTBASE)/Makefile.include
