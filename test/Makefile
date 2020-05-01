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

PROGRAM ?= spidev_test

LIB     ?= $(PROGRAM).so.$(SOVERSION)

OBJS   += spidev_test.o
OBJS   += ../pixtend.o
OBJS   += ../pixtend_2s.o

CFLAGS  += -O3
CFLAGS  += -Wall
CFLAGS  += -DPROGRAM=$(PROGRAM)
CFLAGS  += -I. -I..

CPPFLAGS = $(CFLAGS) -lgcc

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
includedir = $(prefix)/include
libdir = $(prefix)/lib
mandir = $(prefix)/man

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

install: $(ALL)
	install -m 0755 $(PROGRAM)                      $(DESTDIR)$(bindir)
	if which python2; then python2 setup.py install $(PYINSTALLARGS); fi
	if which python3; then python3 setup.py install $(PYINSTALLARGS); fi
ifeq ($(DESTDIR),)
	ldconfig
endif

uninstall:
	rm -f $(DESTDIR)$(bindir)/$(PROGRAM)
	if which python2; then python2 setup.py install $(PYINSTALLARGS) --record /tmp/$(PROGRAM) >/dev/null; sed 's!^!$(DESTDIR)!' < /tmp/pigpio | xargs rm -f >/dev/null; fi
	if which python3; then python3 setup.py install $(PYINSTALLARGS) --record /tmp/$(PROGRAM) >/dev/null; sed 's!^!$(DESTDIR)!' < /tmp/pigpio | xargs rm -f >/dev/null; fi
ifeq ($(DESTDIR),)
	ldconfig
endif

# generated using gcc -MM *.c

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ -MMD -MP -MF $(@:.o=.dep) $<

-include $(wildcard $(OBJS:.o=*.dep))
