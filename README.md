# xtend pi d
xtend pi d is a linux daemon offering simple accessability to a pixtend board

Inspired from:
- https://github.com/joan2937/pigpio
- git://git.code.sf.net/p/pixtend/pxdev
- https://zeromq.org/

Idea is to open a zmq port and then send in a request - response manner.

# Install libzqm

apt-get install libzmq3-dev

# Enable SPI

sudo raspi-config

interfaces
spi

*Do not enable remote gpio!*

