# Add the NimBLE package which provides the bluetooth stack
USEPKG += nimble

# enable the extended advertisement feature of nimble
USEMODULE += nimble_adv_ext

# enable the scanner feature of NimBLE
USEMODULE += nimble_scanner
USEMODULE += nimble_addr

# enable utility features for crafting advertisements
USEMODULE += bluetil_ad

# configure some buffersizes of NimBLE to support larger packet sizes
MSYS_CNT ?= 20
CFLAGS += -DMYNEWT_VAL_BLE_LL_MAX_PKT_SIZE=251
CFLAGS += -DMYNEWT_VAL_BLE_EXT_ADV_MAX_SIZE=1650

# For maximum efficiency, we set the maximum L2CAP fragment size to the same
# value as the maximum link layer packet size.
# WARNING: this value MUST never be larger than MYNEWT_VAL_BLE_LL_MAX_PKT_SIZE
CFLAGS += -DMYNEWT_VAL_BLE_L2CAP_COC_MPS=MYNEWT_VAL_BLE_LL_MAX_PKT_SIZE

# In order to store a full L2CAP fragment/link layer packet in a single block,
# we need to cater for a 48 byte overhead per block.
CFLAGS += -DMYNEWT_VAL_MSYS_1_BLOCK_COUNT=$(MSYS_CNT)
CFLAGS += -DMYNEWT_VAL_MSYS_1_BLOCK_SIZE="(MYNEWT_VAL_BLE_L2CAP_COC_MPS + 48)"