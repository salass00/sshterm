CC    = ppc-amigaos-gcc
STRIP = ppc-amigaos-strip

TARGET  = SSHTerm
VERSION = 1

LIBSSH2DIR = libssh2-1.9.0

OPTIMIZE = -O2
DEBUG    = -g
INCLUDES = -I. -I./$(LIBSSH2DIR)/include -I./libtsm/tsm -I./libtsm/shared
WARNINGS = -Wall -Wwrite-strings -Werror
DEFINES  = -D__NOLIBBASE__ -DOFFSCREEN_BUFFER

# Uncomment to enable debug output
#DEFINES += -DDEBUG

CFLAGS  = $(OPTIMIZE) $(DEBUG) $(INCLUDES) $(WARNINGS) $(DEFINES)
LDFLAGS = -static
LIBS    = 

STRIPFLAGS = -R.comment --strip-unneeded-rel-relocs

SRCS = start.c main.c termwin.c menus.c about.c signal-pid.c term-gc.c \
       bsdsocket-stubs.c amissl-stubs.c zlib-stubs.c malloc.c

OBJS = $(addprefix obj/,$(SRCS:.c=.o))

.PHONY: all
all: $(TARGET)

.PHONY: build-libssh2
build-libssh2:
	$(MAKE) -C $(LIBSSH2DIR) libssh2.a

.PHONY: build-libtsm
build-libtsm:
	$(MAKE) -C libtsm libtsm.a

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIBSSH2DIR)/libssh2.a: build-libssh2
	@true

libtsm/libtsm.a: build-libtsm
	@true

obj/start.o: src/sshterm.h src/term-gc.h $(TARGET)_rev.h
obj/main.o: src/sshterm.h $(TARGET)_rev.h
obj/termwin.o: src/sshterm.h src/term-gc.h $(TARGET)_rev.h
obj/about.o: src/sshterm.h $(TARGET)_rev.h
obj/signal_pid.o: src/sshterm.h
obj/term-gc.o: src/sshterm.h src/term-gc.h libtsm/tsm/libtsm.h
obj/malloc.o: CFLAGS += -fno-builtin

$(TARGET): $(OBJS) libtsm/libtsm.a $(LIBSSH2DIR)/libssh2.a
	$(CC) $(LDFLAGS) -o $@.debug $^ $(LIBS)
	$(STRIP) $(STRIPFLAGS) -o $@ $@.debug

.PHONY: clean
clean:
	$(MAKE) -C $(LIBSSH2DIR) clean
	$(MAKE) -C libtsm clean
	rm -rf $(TARGET) $(TARGET).debug obj

.PHONY: revision
revision:
	bumprev $(VERSION) $(TARGET)

