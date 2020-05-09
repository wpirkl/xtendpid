#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>


#include "app_interface.h"

#include "pixtend.h"
#include "xtendpid.h"


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t pth;
static volatile bool running = true;

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits;
static uint32_t speed;
static uint16_t delay;

static int fd = 0;

struct pixtend pixt;
static union pixtOut tx[2];
static union pixtIn rx[2];
static volatile int buffer_index = 0;

static char model = '2';
static char sub_model = 'S';


static void print_buffer(uint8_t * buffer)
{
    int ret;

    for (ret = 0; ret < sizeof(union pixtOut); ret++) {
            if (!(ret % 8)) puts("");
            printf("%.2X ", buffer[ret]);
    }
    puts("");
}

static void * worker_thread(void * user_data)
{
    int ret = 0;

    size_t transfer_size = pixt_get_transfer_size(&pixt);

    for(;running;)
    {
        pthread_mutex_lock(&mutex);

        int index = buffer_index;

        pixt_prepare_output(&pixt, &tx[index]);

        // here we can do the transfer
        struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)&tx[index],
                .rx_buf = (unsigned long)&rx[index],
                .len = transfer_size,
                .delay_usecs = delay,
                .speed_hz = speed,
                .bits_per_word = bits,
        };

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if(ret > 0)
        {
            if(!pixt_parse_input(&pixt, &rx[index]))
            {
                ret = -1;
            }
            else
            {
                index = (index + 1) & 1;
                buffer_index = index;
                printf("crc error!\n");
            }
        } else {
            printf("SPI transfer error!\n");
        }

        pthread_mutex_unlock(&mutex);

        if(ret <= 0) break;

        // do it every 10 ms
        usleep(10000);
    }

    if(running)
    {
        printf("unexpected end of worker thread! ret is: %d\n", ret);
        running = false;
    }

    return NULL;
}

static int spi_init(void)
{
    int ret = 0;
    uint32_t speed_get;
    uint8_t bits_get;
    uint8_t mode_get;

    memset(&tx, 0, sizeof(tx));
    memset(&rx, 0, sizeof(rx));

    fd = open(device, O_RDWR);
    if (fd < 0) return fd;

    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1) return ret;

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode_get);
    if (ret == -1) return ret;


    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) return ret;

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits_get);
    if (ret == -1)

    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) return ret;

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed_get);
    if (ret == -1) return ret;

    return 0;
}

static void spi_deinit(void)
{
    if(fd > 0)
    {
        close(fd);
    }
}


static void parse_opts(int argc, char * argv[])
{
    // here we could change the pixtend model for example
}


int app_main(int argc, char * argv[])
{

    parse_opts(argc, argv);

    if(!pixt_init(&pixt, model, sub_model))
    {
        printf("unknown model: %c%c\n", model, sub_model);
        return -1;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 0x100000);

    int ret = 0;

    mode = 0;
    speed = 700000;
    bits = 8;
    delay = 0;

    ret = spi_init();
    if(ret < 0)
    {
         printf("spi init failed!\n");
         return ret;
    }

    if(pthread_create(&pth, &attr, worker_thread, NULL) < 0)
    {
        printf("worker_thread pthread_create failed");
        return -1;
    }


    int i;

    for(i=0; i<10 && running; ++i)
    {
        pthread_mutex_lock(&mutex);
        printf("hello from mainloop %d\n", i);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }

    if(!running)
    {
        printf("worker thread died?\n");
    }

    // stop worker thread
    running = false;
    pthread_join(pth, NULL);

    // close spi
    spi_deinit();

    return 0;
}
