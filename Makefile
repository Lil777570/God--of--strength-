TARGET = EBOOT
OBJS = main.o

PSPSDK := $(shell psp-config --pspsdk-path)

CFLAGS = -O2 -G0 -Wall
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PSP_FW_VERSION = 660

LIBS = -lpspgu -lpspgum -lpspdisplay -lpspctrl -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = God of Strength

include $(PSPSDK)/lib/build.mak
