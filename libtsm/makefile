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

$(OBJS): tsm/libtsm-int.h tsm/libtsm.h

libtsm.a: $(OBJS)
	$(AR) -crv $@ $^
	$(RANLIB) $@

.PHONY: clean
clean:
	rm -f libtsm.a $(OBJS)

