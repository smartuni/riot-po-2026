# name of your application
APPLICATION = SenseGate

# If no BOARD is found in the environment, use this default:
BOARD ?= adafruit-feather-nrf52840-sense

PROJECT_BASE ?= $(CURDIR)/../..
RIOTBASE ?= $(PROJECT_BASE)/RIOT
EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules

# Configrue device id and device type.
# This will instruct the key-distro module to include required credentials.
RIOT_CONFIG_DEVICE_ID ?= 1
# Set the device type to 'Gate' to configure some device specific behaviors.
# Dont allow overwrites here, as that would only cause trouble anyway.
# (the corresponding values are defined in tables/types.h)
RIOT_CONFIG_DEVICE_TYPE = DEVICE_TYPE_GATE

# set to 1 to use file based storeage (vfs and littlefs2)
# set to 0 to use mtd/FAL directly (no FS)
USE_FLASHDB_ON_VFS ?= 0

USEMODULE += periph_gpio
USEMODULE += periph_gpio_irq

# Enable the milliseconds timer.
USEMODULE += ztimer
USEMODULE += ztimer_msec
USEMODULE += ztimer_sec

USEPKG += u8g2
USEMODULE += event
USEMODULE += event_thread_highest
USEMODULE += event_thread_medium
USEMODULE += event_thread_lowest
USEMODULE += event_thread
USEMODULE += event_timeout
USEMODULE += mate_lorawan
USEMODULE += mate_ble
USEMODULE += inductive_sensor

CFLAGS += -DRIOT_CONFIG_DEVICE_ID=$(RIOT_CONFIG_DEVICE_ID)
CFLAGS += -DRIOT_CONFIG_DEVICE_TYPE=$(RIOT_CONFIG_DEVICE_TYPE)
CFLAGS += -DEVENT_THREAD_HIGHEST_STACKSIZE=6*1024
CFLAGS += -DEVENT_THREAD_MEDIUM_STACKSIZE=6*1024

ifeq ($(RIOT_CONFIG_DEVICE_ID),1)
CFLAGS += -DGATE_OBSERVER_DISTANCE_SENSOR_CNT=1
else
CFLAGS += -DGATE_OBSERVER_DISTANCE_SENSOR_CNT=0
endif

CFLAGS += -DTHREAD_STACKSIZE_MAIN=THREAD_STACKSIZE_LARGE*2

# RAM-based storage backend
USEMODULE += mtd_emulated

# credential manager stuff
USEMODULE += credential_manager
USEMODULE += flashdb_store_service

# configure ram mtd to a size of 64 kByte
CFLAGS += -DRAM_MTD_SECTOR_COUNT=\(16\)
CFLAGS += -DRAM_MTD_PAGE_PER_SECTOR=\(64\)
CFLAGS += -DRAM_MTD_PAGE_SIZE=\(64\)

ifeq ($(USE_FLASHDB_ON_VFS),1)
# use two sectors of the storage for each flashdb (PER STORAGE SERVICE)
# TODO: make configurable for each store service separately (?)
CFLAGS += -DFLASHDB_STORE_SERVICE_SECTOR_SIZE=\(RAM_MTD_PAGE_PER_SECTOR*RAM_MTD_PAGE_SIZE\)
CFLAGS += -DFLASHDB_STORE_SERVICE_DB_SIZE=\(\(FLASHDB_STORE_SERVICE_SECTOR_SIZE*2\)\)
USEMODULE += vfs_default
USEMODULE += vfs_auto_format
else
# when using mtd directly we don't need to keep free space for the FS.
# We just assign each DB the size of one partition.
CFLAGS += -DFLASHDB_STORE_SERVICE_SECTOR_SIZE=\(RAM_MTD_PAGE_PER_SECTOR*RAM_MTD_PAGE_SIZE\)
CFLAGS += -DFLASHDB_STORE_SERVICE_DB_SIZE=\(\(FLASHDB_STORE_SERVICE_SECTOR_SIZE*RAM_MTD_SECTOR_COUNT/2\)\)
USEMODULE += flashdb_mtd
CFLAGS += -DFAL_PART0_LABEL=\"/cred\"
CFLAGS += -DFAL_PART0_LENGTH=FLASHDB_STORE_SERVICE_DB_SIZE
CFLAGS += -DFAL_PART1_LABEL=\"/tables\"
CFLAGS += -DFAL_PART1_LENGTH=FLASHDB_STORE_SERVICE_DB_SIZE
endif

# tables stuff
USEMODULE += tables
USEMODULE += tables_shell
USEMODULE += cose_crypto_service
USEMODULE += hlc_ztimer

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Some workarounds are needed in order to get the tutorial running on
# some computers.
PREFLASH_DELAY=8

# for debugging
USEMODULE += ps
USEMODULE += shell_cmd_genfile
USEMODULE += shell_cmds_default

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1
CFLAGS += -DDEBUG_ASSERT_VERBOSE
CFLAGS += -I$(APPDIR)/include
include $(RIOTBASE)/Makefile.include
