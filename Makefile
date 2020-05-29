CFLAGS += -O3 -std=c99 -Wall -Wextra -pedantic -Wold-style-declaration
CFLAGS += -Wmissing-prototypes -Wno-unused-parameter
LDFLAGS = -static
LDLIBS = `pkg-config $(LDFLAGS) --libs x11`
CC     ?= gcc

all install: sowm
	@-usage="$(install)"; install -Dm755 $< $${usage:?make install=BINDIR}/sowm
config.h:
	cp config.def.h config.h
sowm.o: sowm.c config.h Makefile
clean:; rm -f sowm *.o
.PHONY: all install clean
