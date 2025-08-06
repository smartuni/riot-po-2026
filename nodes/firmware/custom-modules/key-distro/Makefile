MODULE := key-distro

INCLUDES += -I$(CURDIR)/include

ifeq ($(DEVICE_TYPE),1)
        INCLUDES += -I$(CURDIR)/ttn_configs/sensemate-$(RIOT_CONFIG_DEVICE_ID).mk
else
        INCLUDES += -I$(CURDIR)/ttn_configs/sensegate-$(RIOT_CONFIG_DEVICE_ID).mk
endif

include $(RIOTBASE)/Makefile.base