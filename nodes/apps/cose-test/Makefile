# This has to be the absolute path to the RIOT base directory:
APPLICATION = my_app
PROJECT_BASE ?= $(CURDIR)/../..
RIOTBASE ?= $(PROJECT_BASE)/RIOT

EXTERNAL_MODULE_DIRS += $(PROJECT_BASE)/custom-modules

# If no BOARD is found in the environment, use this default:
BOARD ?= adafruit-feather-nrf52840-sense

# Eigene Module aktivieren
USEMODULE += cose-service

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Some workarounds are needed in order to get the tutorial running on
# some computers.
-include ../../lab_workarounds.mk

# This board requires a start sleep to actually catch the printed output
USEMODULE += xtimer
USEMODULE += ztimer_sec
USEMODULE += benchmark
USEPKG += tinycbor
USEPKG += libcose
USEPKG += libsodium
USEMODULE += libcose_crypt_sodium

CFLAGS += '-DTHREAD_STACKSIZE_MAIN=(THREAD_STACKSIZE_DEFAULT + 1024)'


# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
