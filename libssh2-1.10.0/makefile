CC     = ppc-amigaos-gcc
AR     = ppc-amigaos-ar
RANLIB = ppc-amigaos-ranlib

OPTIMIZE = -O2
DEBUG    = -g
WARNINGS = -Wall -Wwrite-strings -Werror -Wno-deprecated-declarations
INCLUDES = -I./include -I./src
DEFINES  = -DLIBSSH2_OPENSSL -DLIBSSH2_DH_GEX_NEW -DLIBSSH2_HAVE_ZLIB -D__USE_INLINE__
#DEFINES += -DLIBSSH2DEBUG

CFLAGS  = --std=gnu99 $(OPTIMIZE) $(DEBUG) $(WARNINGS) $(INCLUDES) $(DEFINES)

SRCS = agent.c bcrypt_pbkdf.c blowfish.c channel.c comp.c crypt.c global.c \
       hostkey.c keepalive.c kex.c knownhost.c libgcrypt.c mac.c mbedtls.c \
       misc.c openssl.c packet.c pem.c publickey.c scp.c session.c sftp.c \
       transport.c userauth.c version.c wincng.c

OBJS = $(addprefix obj/,$(SRCS:.c=.o))

.PHONY: all
all: libssh2.a

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $<

libssh2.a: $(OBJS)
	$(AR) -crv $@ $^
	$(RANLIB) $@

.PHONY: clean
clean:
	rm -rf libssh2.a obj

