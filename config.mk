PREFIX = /usr
MANPREFIX = $(PREFIX)/share/man

CC = c99

CPPFLAGS = -D_DEFAULT_SOURCE -D_GNU_SOURCE
CFLAGS   = -Wall -O2
LDFLAGS  = -s
