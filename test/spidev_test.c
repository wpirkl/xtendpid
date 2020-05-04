/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "../pixtend.h"
#include "../pixtend_2s.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))


static int wr_gpio(const char * filename, const char * value) {

	int fd;
	int res;

	fd = open(filename, O_WRONLY);
	if(fd < 0)
	{
		return fd;
	}

	res = write(fd, value, strlen(value));

	close(fd);

	return res;
}

static int rd_gpio(const char * filename, char * value, const size_t len) {

        int fd;
        int res;

        fd = open(filename, O_RDONLY);
        if(fd < 0)
        {
                return fd;
        }

        res = read(fd, value, len);

        close(fd);

        return res;
}




int gpio_export(int pin)
{
	char * filename = "/sys/class/gpio/export";
	char cmd[3];

	snprintf(cmd, sizeof(cmd), "%d", pin);

	return wr_gpio(filename, cmd);
}

int gpio_unexport(int pin)
{
        char * filename = "/sys/class/gpio/unexport";
        char cmd[3];

        snprintf(cmd, sizeof(cmd), "%d", pin);

        return wr_gpio(filename, cmd);
}

int gpio_direction(int pin, bool output)
{
	char filename[64];
	char * direction;

	snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/direction", pin);

	if(output) {
		direction = "out";
	} else {
		direction = "in";
	}

	return wr_gpio(filename, direction);
}

int gpio_read(int pin)
{
        char filename[64];
        char result[8];
	int res;

        snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/value", pin);

	res = rd_gpio(filename, result, sizeof(result)-1);
	if(res < 0) return res;

	result[res] = '\0';

	return atoi(result);
}

int gpio_write(int pin, bool on)
{
        char filename[64];
        char * value;

        snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/value", pin);

        if(on) {
                value = "1";
        } else {
                value = "0";
        }

        return wr_gpio(filename, value);
}




static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 700000;
static uint16_t delay;

static void transfer(int fd)
{
	int ret;
	union pixtOut tx;
	union pixtIn rx;

	printf("size of out: %d\n", sizeof(tx));
        printf("size of in: %d\n", sizeof(rx));


	memset(&tx, 0, sizeof(tx));
	memset(&rx, 0, sizeof(rx));

	pixtend_v2s_prepare_output(&tx);

	printf("sending:\n");

        for (ret = 0; ret < sizeof(rx); ret++) {
                if (!(ret % 8))
                        puts("");
                printf("%.2X ", ((uint8_t*)&rx)[ret]);
        }
        puts("\n");


	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&tx,
		.rx_buf = (unsigned long)&rx,
		.len = sizeof(rx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

	pixtend_v2s_parse_input(&rx);

        printf("received:\n");

	for (ret = 0; ret < sizeof(rx); ret++) {
		if (!(ret % 8))
			puts("");
		printf("%.2X ", ((uint8_t*)&rx)[ret]);
	}
	puts("");
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev0.0)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	parse_opts(argc, argv);

	// export gpio
//	ret = gpio_export(24);
	if(ret < 0)
		pabort("can't export GPIO24");

//        ret = gpio_export(23);
        if(ret < 0)
                pabort("can't export GPIO23");

//        ret = gpio_direction(24, true);
        if(ret < 0)
                pabort("can't set GPIO24 to output");

//        ret = gpio_direction(23, true);
        if(ret < 0)
                pabort("can't set GPIO23 to output");

//        ret = gpio_write(24, true);
        if(ret < 0)
                pabort("can't set GPIO24 to high");

//        ret = gpio_write(23, true);
        if(ret < 0)
                pabort("can't set GPIO23 to high");

//        ret = gpio_write(23, false);
        if(ret < 0)
                pabort("can't set GPIO23 to low");


	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	transfer(fd);

	close(fd);

//        ret = gpio_unexport(24);
        if(ret < 0)
                pabort("can't unexport GPIO24");

//        ret = gpio_unexport(23);
        if(ret < 0)
                pabort("can't unexport GPIO23");


	return ret;
}
