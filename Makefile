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


OBJ_MAN3 =\
	libcoopgamma_async_context_destroy.o\
	libcoopgamma_async_context_initialise.o\
	libcoopgamma_async_context_marshal.o\
	libcoopgamma_async_context_unmarshal.o\
	libcoopgamma_connect.o\
	libcoopgamma_context_destroy.o\
	libcoopgamma_context_initialise.o\
	libcoopgamma_context_marshal.o\
	libcoopgamma_context_unmarshal.o\
	libcoopgamma_crtc_info_destroy.o\
	libcoopgamma_crtc_info_initialise.o\
	libcoopgamma_crtc_info_marshal.o\
	libcoopgamma_crtc_info_unmarshal.o\
	libcoopgamma_error_destroy.o\
	libcoopgamma_error_initialise.o\
	libcoopgamma_error_marshal.o\
	libcoopgamma_error_unmarshal.o\
	libcoopgamma_filter_destroy.o\
	libcoopgamma_filter_initialise.o\
	libcoopgamma_filter_marshal.o\
	libcoopgamma_filter_query_destroy.o\
	libcoopgamma_filter_query_initialise.o\
	libcoopgamma_filter_query_marshal.o\
	libcoopgamma_filter_query_unmarshal.o\
	libcoopgamma_filter_table_destroy.o\
	libcoopgamma_filter_table_initialise.o\
	libcoopgamma_filter_table_marshal.o\
	libcoopgamma_filter_table_unmarshal.o\
	libcoopgamma_filter_unmarshal.o\
	libcoopgamma_flush.o\
	libcoopgamma_get_crtcs_recv.o\
	libcoopgamma_get_crtcs_send.o\
	libcoopgamma_get_crtcs_sync.o\
	libcoopgamma_get_gamma_info_recv.o\
	libcoopgamma_get_gamma_info_send.o\
	libcoopgamma_get_gamma_info_sync.o\
	libcoopgamma_get_gamma_recv.o\
	libcoopgamma_get_gamma_send.o\
	libcoopgamma_get_gamma_sync.o\
	libcoopgamma_get_method_and_site.o\
	libcoopgamma_get_methods.o\
	libcoopgamma_get_pid_file.o\
	libcoopgamma_get_socket_file.o\
	libcoopgamma_queried_filter_destroy.o\
	libcoopgamma_queried_filter_initialise.o\
	libcoopgamma_queried_filter_marshal.o\
	libcoopgamma_queried_filter_unmarshal.o\
	libcoopgamma_ramps_destroy.o\
	libcoopgamma_set_gamma_recv.o\
	libcoopgamma_set_gamma_send.o\
	libcoopgamma_set_gamma_sync.o\
	libcoopgamma_set_nonblocking.o\
	libcoopgamma_skip_message.o\
	libcoopgamma_synchronise.o


OBJ =\
	$(OBJ_MAN3)\
	libcoopgamma.o\
	libcoopgamma_check_error__.o\
	libcoopgamma_query__.o\
	libcoopgamma_ramps_initialise_.o\
	libcoopgamma_ramps_marshal_.o\
	libcoopgamma_ramps_unmarshal_.o\
	libcoopgamma_send_message__.o

HDR =\
	libcoopgamma.h\
	common.h

LOBJ = $(OBJ:.o=.lo)


MAN0 =\
	libcoopgamma.h.0

MAN3 =\
	$(OBJ_MAN3:.o=.3)\
	libcoopgamma_ramps_initialise.3\
	libcoopgamma_ramps_marshal.3\
	libcoopgamma_ramps_unmarshal.3

MAN7 =\
	libcoopgamma.7


all: libcoopgamma.a libcoopgamma.$(LIBEXT) test
$(OBJ): $(HDR)
$(LOBJ): $(HDR)

.c.o:
	$(CC) -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

.c.lo:
	$(CC) -fPIC -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

libcoopgamma.a: $(OBJ)
	@rm -f -- $@
	$(AR) rc $@ $(OBJ)
	$(AR) s $@

libcoopgamma.$(LIBEXT): $(LOBJ)
	$(CC) $(LIBFLAGS) -o $@ $(LOBJ) $(LDFLAGS)

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
