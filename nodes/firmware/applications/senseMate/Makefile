# name of your application
APPLICATION = SenseMate
# If no BOARD is found in the environment, use this default:
BOARD ?= adafruit-feather-nrf52840-sense

PROJECT_BASE ?= $(CURDIR)/../..
RIOTBASE ?= $(PROJECT_BASE)/RIOT
EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1
VERBOSE_ASSERT ?= 1

# Configrue device id and device type.
# This will instruct the key-distro module to include required credentials.
RIOT_CONFIG_DEVICE_ID ?= 1
# Set the device type to SenseMate to configure some device specific behaviors.
# Dont allow overwrites here, as that would only cause trouble anyway.
# (the corresponding values are defined in tables/types.h)
RIOT_CONFIG_DEVICE_TYPE = DEVICE_TYPE_SENSEMATE
# Some workarounds are needed in order to get the tutorial running on
# some computers.
-include ../../lab_workarounds.mk

# This board requires a start sleep to actually catch the printed output
USEMODULE += ztimer_sec

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
USEMODULE += tables_shell

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
USEPKG += u8g2
USEMODULE += periph_i2c
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
DIRS += sensemate_ui
USEMODULE += sensemate_ui
USEMODULE += ps
USEMODULE += shell_cmds_default

# RAM-based storage backend
USEMODULE += mtd_emulated

# credential manager stuff
USEMODULE += credential_manager
USEMODULE += flashdb_store_service
USEMODULE += vfs_default
USEMODULE += vfs_auto_format

# configure ram mtd to a size of 8 kByte
CFLAGS += -DRAM_MTD_SECTOR_COUNT=\(16\)
CFLAGS += -DRAM_MTD_PAGE_PER_SECTOR=\(64\)
CFLAGS += -DRAM_MTD_PAGE_SIZE=\(64\)

# use two sectors of the storage for flashdb (PER STORAGE SERVICE)
# TODO: make configurable for each store sercvice separately
CFLAGS += -DFLASHDB_STORE_SERVICE_SECTOR_SIZE=\(RAM_MTD_PAGE_PER_SECTOR*RAM_MTD_PAGE_SIZE\)
CFLAGS += -DFLASHDB_STORE_SERVICE_DB_SIZE=\(\(FLASHDB_STORE_SERVICE_SECTOR_SIZE*2\)\)

# tables stuff
USEMODULE += tables
USEMODULE += cose_crypto_service
USEMODULE += hlc_ztimer


# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

CFLAGS += -I$(APPDIR)/include
include $(RIOTBASE)/Makefile.include
