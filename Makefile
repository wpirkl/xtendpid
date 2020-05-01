

PROGRAM ?= ../xtendpid

OBJS += ../xtendpid.o
OBJS += ../pixtend.o

CFLAGS += -I..


export PROGRAM
export OBJS
export CFLAGS


all:
	$(MAKE) -C linux_daemon all

clean:
	$(MAKE) -C linux_daemon clean

