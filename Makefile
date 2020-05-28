#
# Set CROSS_PREFIX to prepend to all compiler tools at once for easier
# cross-compilation.
CROSS_PREFIX ?=
CC           = $(CROSS_PREFIX)gcc
AR           = $(CROSS_PREFIX)ar
RANLIB       = $(CROSS_PREFIX)ranlib
SIZE         = $(CROSS_PREFIX)size
STRIP        = $(CROSS_PREFIX)strip
SHLIB        = $(CC) -shared
STRIPLIB     = $(STRIP) --strip-unneeded

SOVERSION ?= 1

PROGRAM ?= xtendpid

LIB     ?= $(PROGRAM).so.$(SOVERSION)

OBJS   += linux_daemon/linux_daemon.o
OBJS   += linux_daemon/app_interface.o
OBJS   += xtendpid.o
OBJS   += pixtend.o
OBJS   += pixtend_2s.o

CFLAGS  += -O3
CFLAGS  += -Wall
CFLAGS  += -DPROGRAM=$(PROGRAM)
CFLAGS  += -I. -Ilinux_daemon
CFLAGS  += -pthread
CFLAGS  += -lzmq

CPPFLAGS = $(CFLAGS) -lgcc

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
includedir = $(prefix)/include
libdir = $(prefix)/lib
mandir = $(prefix)/man
servicedir = /etc/systemd/system

all: $(PROGRAM)

lib: $(LIB)

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS) $(CFLAGS)
	$(STRIP) $(PROGRAM)

clean:
	rm -f $(LIB) $(PROGRAM) $(OBJS) $(OBJS:.o=.dep)

ifeq ($(DESTDIR),)
  PYINSTALLARGS =
else
  PYINSTALLARGS = --root=$(DESTDIR)
endif

install: $(PROGRAM)
	install -m 0755 $(PROGRAM)           $(DESTDIR)$(bindir)
	install -m 0755 util/xtendpid_daemon $(DESTDIR)$(bindir)
	install util/xtendpid_daemon.service $(DESTDIR)$(servicedir)
ifeq ($(DESTDIR),)
#	ldconfig
endif

uninstall:
	rm -f $(DESTDIR)$(bindir)/$(PROGRAM)
	rm -f $(DESTDIR)$(bindir)/xtendpid_daemon
	rm -f $(DESTDIR)$(servicedir)/xtendpid_daemon.service
ifeq ($(DESTDIR),)
#	ldconfig
endif

# generated using gcc -MM *.c

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ -MMD -MP -MF $(@:.o=.dep) $<

-include $(wildcard $(OBJS:.o=*.dep))
