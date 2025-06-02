# name of your application
APPLICATION = senseMate

# If no BOARD is found in the environment, use this default:
BOARD ?= feather-nrf52840-sense

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Some workarounds are needed in order to get the tutorial running on
# some computers.
-include ../lab_workarounds.mk

# This board requires a start sleep to actually catch the printed output
USEMODULE += ztimer_sec

# Include other packages here e.g.
# include ../../other_package/other_package.inc.mk

# We use shell in this example
USEMODULE += shell

USEMODULE += mate_lorawan

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1


# ------------ [Task 1.9] --------------------
# Replace these values with your own from the registered device
# No spaces between the quotes
CFLAGS += -DCONFIG_LORAMAC_APP_EUI_DEFAULT=\"0000000000000000\"
CFLAGS += -DCONFIG_LORAMAC_DEV_EUI_DEFAULT=\"70B3D57ED0070C99\"
CFLAGS += -DCONFIG_LORAMAC_APP_KEY_DEFAULT=\"D5586D89F62687A027BD6D3273256FCB\"
# -----------------------------------------

# include SAUL to interact with onboard sensors and actuators
USEMODULE += saul

# include and auto-initialize all available sensors
USEMODULE += saul_default

# Include board's default network devices and auto-initialization of GNRC
# interfaces
USEMODULE += netdev_default
USEMODULE += auto_init_gnrc_netif

# GNRC modules to build and receive packets
USEMODULE += gnrc_pktbuf
USEMODULE += gnrc_netif_hdr
USEMODULE += gnrc_netapi
USEMODULE += gnrc_netreg
USEMODULE += gnrc_neterr

# Add support for GNRC LoRaWAN
USEMODULE += gnrc_lorawan

# Include OD to print received packets
USEMODULE += od

# we use ztimer to read sensors periodically
USEMODULE += ztimer
USEMODULE += ztimer_sec

# We want to use OTAA join procedure
CFLAGS += -DCONFIG_LORAMAC_DEFAULT_JOIN_PROCEDURE_OTAA

# Bring the LoRa radio driver
USEMODULE += sx127x

# # Indicate how we connected the LoRa radio to the microcontroller
# CFLAGS += -DSX127X_PARAM_SPI_NSS=GPIO_PIN\(0,27\) #B
# CFLAGS += -DSX127X_PARAM_RESET=GPIO_PIN\(0,26\) #C
# CFLAGS += -DSX127X_PARAM_DIO0=GPIO_PIN\(0,6\) #A
# CFLAGS += -DSX127X_PARAM_DIO1=GPIO_PIN\(1,8\) #E
# CFLAGS += -DSX127X_PARAM_DIO2=GPIO_PIN\(0,7\) #D
# CFLAGS += -DSX127X_PARAM_DIO3=GPIO_UNDEF
# CFLAGS += -DSX127X_PARAM_DIO4=GPIO_UNDEF
# CFLAGS += -DSX127X_PARAM_DIO5=GPIO_UNDEF
# CFLAGS += -DSX127X_PARAM_PASELECT=SX127X_PA_BOOST

EXTERNAL_MODULE_DIRS=../../

include $(RIOTBASE)/Makefile.include
