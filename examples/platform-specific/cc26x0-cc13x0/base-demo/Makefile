CONTIKI_PROJECT = base-demo

CONTIKI = ../../../..
include $(CONTIKI)/Makefile.identify-target

MODULES_REL += $(TARGET)
## simplelink support
PLATFORMS_ONLY = cc26x0-cc13x0 simplelink

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include
