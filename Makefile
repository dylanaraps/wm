.POSIX:

PREFIX = /usr/local

ALL_WARN    = -Wall -Wextra -pedantic -Wmissing-prototypes -Wstrict-prototypes
ALL_CFLAGS  = $(CFLAGS) $(CPPFLAGS) -std=c99 $(ALL_WARN)
ALL_LDFLAGS = $(LDFLAGS) $(LIBS) -lxcb -lxcb-keysyms

CC = cc

OBJ = src/sowm.o
HDR =

.c.o:
	$(CC) $(ALL_CFLAGS) -c -o $@ $<

sowm: $(OBJ)
	$(CC) $(ALL_CFLAGS) -o $@ $(OBJ) $(ALL_LDFLAGS)

$(OBJ): $(HDR)

install: sowm
	mkdir -p $(DESTDIR)/bin
	cp sowm $(DESTDIR)/bin/sowm

uninstall:
	rm -f $(DESTDIR)/bin/sowm

clean:
	rm -f sowm src/*.o

.PHONY: install uninstall clean
