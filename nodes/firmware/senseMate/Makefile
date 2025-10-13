# name of your application
APPLICATION = SenseMate
# If no BOARD is found in the environment, use this default:
BOARD ?= adafruit-feather-nrf52840-sense

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

PROJECT_BASE ?= $(CURDIR)/../
EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules


# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1
VERBOSE_ASSERT ?= 1

# Configrue device id and device type.
# This will instruct the key-distro module to include required credentials.
RIOT_CONFIG_DEVICE_ID ?= 1
RIOT_CONFIG_DEVICE_TYPE ?= SENSEMATE_NODE

# Some workarounds are needed in order to get the tutorial running on
# some computers.
-include ../../lab_workarounds.mk

# This board requires a start sleep to actually catch the printed output
USEMODULE += ztimer_sec

USEPKG += u8g2
USEMODULE += periph_i2c

USEMODULE += periph_gpio
USEMODULE += periph_gpio_irq

USEMODULE += ztimer
USEMODULE += ztimer_msec
USEMODULE += ztimer_usec

USEMODULE += event
USEMODULE += event_thread_highest
USEMODULE += event_thread
USEMODULE += event_timeout

USEMODULE += tables

USEMODULE += mate_lorawan
USEMODULE += mate_ble

PREFLASH_DELAY=8

CFLAGS += -DRIOT_CONFIG_DEVICE_ID=$(RIOT_CONFIG_DEVICE_ID)
CFLAGS += -DRIOT_CONFIG_DEVICE_TYPE=$(RIOT_CONFIG_DEVICE_TYPE)
CFLAGS += -DI2C_DEVICE_IDX=0
CFLAGS += -DI2C_DISPLAY_ADDR=0x3c
CFLAGS += -DU8G2_DISPLAY_INIT_FUNC=u8g2_Setup_ssd1306_i2c_128x64_noname_f
CFLAGS += -DEVENT_THREAD_HIGHEST_STACKSIZE=1024*3
CFLAGS += -DEVENT_THREAD_MEDIUM_STACKSIZE=1024*3

# stuff for new lvgl-based menu implementation
USEPKG += lvgl
USEMODULE += lvgl_contrib
USEMODULE += lvgl_widget_slider
USEMODULE += lvgl_extra_widget_chart
USEMODULE += lvgl_extra_widget_win
USEMODULE += lvgl_extra_widget_list
USEMODULE += lvgl_extra_widget_tileview
USEMODULE += lvgl_extra_layout_flex
USEMODULE += lvgl_extra_theme_mono
USEMODULE += u8g2_disp_dev
CFLAGS += -DTHREAD_STACKSIZE_MAIN=2*4096
CFLAGS += -DTHREAD_STACKSIZE_UI=2*4096
CFLAGS += -DLV_MEM_SIZE=1024*16
DIRS += ui
USEMODULE += ui

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
