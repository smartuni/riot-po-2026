# name of your application
APPLICATION = displayTest

# If no BOARD is found in the environment, use this default:
BOARD ?= adafruit-feather-nrf52840-sense

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../../RIOT
PROJECT_BASE ?= $(CURDIR)/../../../
EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1
VERBOSE_ASSERT ?= 1
# Some workarounds are needed in order to get the tutorial running on
# some computers.
-include ../../../lab_workarounds.mk

# This board requires a start sleep to actually catch the printed output
USEMODULE += ztimer_sec

USEPKG += u8g2
USEMODULE += periph_i2c

USEMODULE += periph_gpio
USEMODULE += periph_gpio_irq

USEMODULE += ztimer
USEMODULE += ztimer_msec

USEMODULE += event
USEMODULE += event_thread_highest
USEMODULE += event_thread
USEMODULE += event_timeout

USEMODULE += tables

USEMODULE += mate_lorawan



CFLAGS += -DI2C_DEVICE_IDX=0
CFLAGS += -DI2C_DISPLAY_ADDR=0x3c
CFLAGS += -DU8G2_DISPLAY_INIT_FUNC=u8g2_Setup_ssd1306_i2c_128x64_noname_f
CFLAGS += -DEVENT_THREAD_HIGHEST_STACKSIZE=1024


# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
