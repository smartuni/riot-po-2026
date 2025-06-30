# name of your application
APPLICATION = logic

# If no BOARD is found in the environment, use this default:
BOARD ?= feather-nrf52840-sense

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../../RIOT
PROJECT_BASE ?= $(CURDIR)/../../../
EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules

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
USEMODULE += tables
USEMODULE += mate_lorawan
USEMODULE += mate_ble


CFLAGS += -DEVENT_THREAD_HIGHEST_STACKSIZE=1024

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Some workarounds are needed in order to get the tutorial running on
# some computers.
PREFLASH_DELAY=10


# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1
CFLAGS += -DDEBUG_ASSERT_VERBOSE
include $(RIOTBASE)/Makefile.include