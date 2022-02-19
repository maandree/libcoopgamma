.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

OS = linux
# Linux:   linux
# Mac OS:  macos
# Windows: windows
include mk/$(OS).mk


LIB_MAJOR = 1
LIB_MINOR = 2
LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR)


include man.mk


all: libcoopgamma.a libcoopgamma.$(LIBEXT) test

.c.o:
	$(CC) -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

.c.lo:
	$(CC) -fPIC -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

libcoopgamma.a: libcoopgamma.o
	$(AR) rc $@ $?
	$(AR) s $@

libcoopgamma.$(LIBEXT): libcoopgamma.lo
	$(CC) $(LIBFLAGS) -o $@ libcoopgamma.lo $(LDFLAGS)

test: test.o libcoopgamma.a
	$(CC) -o $@ test.o libcoopgamma.a $(LDFLAGS)

check: test
	./test

install: libcoopgamma.a libcoopgamma.$(LIBEXT)
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man0"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man3"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man7"
	cp -- libcoopgamma.h "$(DESTDIR)$(PREFIX)/include"
	cp -- libcoopgamma.a "$(DESTDIR)$(PREFIX)/lib"
	cp -- libcoopgamma.$(LIBEXT) "$(DESTDIR)$(PREFIX)/lib/libcoopgamma.$(LIBMINOREXT)"
	$(FIX_INSTALL_NAME) "$(DESTDIR)$(PREFIX)/lib/libcoopgamma.$(LIBMINOREXT)"
	ln -sf -- libcoopgamma.$(LIBMINOREXT) "$(DESTDIR)$(PREFIX)/lib/libcoopgamma.$(LIBMAJOREXT)"
	ln -sf -- libcoopgamma.$(LIBMINOREXT) "$(DESTDIR)$(PREFIX)/lib/libcoopgamma.$(LIBEXT)"
	cp -- $(MAN0) "$(DESTDIR)$(MANPREFIX)/man0"
	cp -- $(MAN3) "$(DESTDIR)$(MANPREFIX)/man3"
	cp -- $(MAN7) "$(DESTDIR)$(MANPREFIX)/man7"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libcoopgamma.h"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcoopgamma.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcoopgamma.$(LIBMINOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcoopgamma.$(LIBMAJOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcoopgamma.$(LIBEXT)"
	-cd -- "$(DESTDIR)$(MANPREFIX)/man0/" && rm -f -- $(MAN0)
	-cd -- "$(DESTDIR)$(MANPREFIX)/man3/" && rm -f -- $(MAN3)
	-cd -- "$(DESTDIR)$(MANPREFIX)/man7/" && rm -f -- $(MAN7)

clean:
	-rm -f -- *.a *.lo *.o *.su *.$(LIBEXT) test

.SUFFIXES:
.SUFFIXES: .lo .o .c

.PHONY: all check install uninstall clean
