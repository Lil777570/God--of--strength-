TARGET = EBOOT
OBJS = main.o

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PSP_FW_VERSION = 660

LIBS = -lpspgu -lpspgum -lpspdisplay -lpspctrl -lpspkernel -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = God of Strength

include $(PSPSDK)/lib/build.mak
