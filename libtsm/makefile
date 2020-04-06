CC     = ppc-amigaos-gcc
AR     = ppc-amigaos-ar
RANLIB = ppc-amigaos-ranlib

OPTIMIZE = -O2
DEBUG    = -g
WARNINGS = -Wall -Wwrite-strings -Werror
INCLUDES = -I. -I./external -I./shared
DEFINES  = 

CFLAGS  = $(OPTIMIZE) $(DEBUG) $(WARNINGS) $(INCLUDES) $(DEFINES)

SRCS = tsm/tsm-render.c \
       tsm/tsm-screen.c \
       tsm/tsm-selection.c \
       tsm/tsm-unicode.c \
       tsm/tsm-vte.c \
       tsm/tsm-vte-charsets.c \
       shared/shl-htable.c \
       shared/shl-ring.c \
       external/wcwidth/wcwidth.c

OBJS = $(SRCS:.c=.o)

.PHONY: all
all: libtsm.a

tsm/tsm-render.o: tsm/libtsm.h tsm/libtsm-int.h shared/shl-llog.h
tsm/tsm-screen.o: tsm/libtsm.h tsm/libtsm-int.h shared/shl-llog.h
tsm/tsm-selection.o: tsm/libtsm.h tsm/libtsm-int.h shared/shl-llog.h
tsm/tsm-unicode.o: tsm/libtsm.h tsm/libtsm-int.h shared/shl-array.h shared/shl-htable.h \
                   external/wcwidth/wcwidth.h
tsm/tsm-vte.o: tsm/libtsm.h tsm/libtsm-int.h shared/shl-llog.h \
               tsm/tsm-vte-keyboard-amiga.c external/xkbcommon/xkbcommon-keysyms.h
tsm/tsm-vte-charsets.o: tsm/libtsm.h tsm/libtsm-int.h

shared/shl-htable.o: shared/shl-htable.h
shared/shl-ring.o: shared/shl-ring.h shared/shl-macro.h

external/wcwidth/wcwidth.o: external/wcwidth/wcwidth.h

libtsm.a: $(OBJS)
	$(AR) -crv $@ $^
	$(RANLIB) $@

.PHONY: clean
clean:
	rm -f libtsm.a $(OBJS)

