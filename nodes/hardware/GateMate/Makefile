# name of your application
APPLICATION = senseMate

# If no BOARD is found in the environment, use this default:
BOARD ?= feather-nrf52840-sense

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Some workarounds are needed in order to get the tutorial running on
# some computers.
-include ../lab_workarounds.mk

# This board requires a start sleep to actually catch the printed output
USEMODULE += ztimer_sec

# We use shell in this example
USEMODULE += shell

# Use our custom mate_ble module
USEMODULE += mate_ble

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

EXTERNAL_MODULE_DIRS += ../../

include $(RIOTBASE)/Makefile.include