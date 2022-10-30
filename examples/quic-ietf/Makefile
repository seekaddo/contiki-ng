CONTIKI_PROJECT = quic-example-server
all: $(CONTIKI_PROJECT)

# Do not try to build on Sky because of code size limitation
PLATFORMS_EXCLUDE = sky z1


# just for testing
MODULES += os/services/shell

CONTIKI=../..
#CONTIKI=/home/seekaddo/project-omega/contiki-ng

MAKE_QUIC = 3
#MAKE_NET = MAKE_NET_IPV6
#MAKE_MAC = MAKE_MAC_BLE
# Include the CoAP implementation
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/quic-en

include $(CONTIKI)/Makefile.include
