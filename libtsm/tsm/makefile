CC     = ppc-amigaos-gcc
AR     = ppc-amigaos-ar
RANLIB = ppc-amigaos-ranlib

OPTIMIZE = -O2
DEBUG    = -g
WARNINGS = -Wall -Wwrite-strings -Werror
INCLUDES = 
DEFINES  = 

CFLAGS  = $(OPTIMIZE) $(DEBUG) $(WARNINGS) $(INCLUDES) $(DEFINES)

SRCS = tsm-render.c tsm-screen.c tsm-selection.c tsm-unicode.c tsm-vte.c \
       tsm-vte-charsets.c

OBJS = $(SRCS:.c=.o)

.PHONY: all
all: libtsm.a

$(OBJS): libtsm-int.h libtsm.h

libtsm.a: $(OBJS)
	$(AR) -crv $@ $^
	$(RANLIB) $@

.PHONY: clean
clean:
	rm -f libtsm.a $(OBJS)

