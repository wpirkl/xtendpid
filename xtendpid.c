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

#include <zmq.h>

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

static uint16_t port = 5555;
static char address[256] = "*";


static void print_buffer(uint8_t * buffer, size_t len)
{
    int ret;

    for (ret = 0; ret < len; ++ret) {
            if (!(ret % 8) && ret > 0) puts("");
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
                // print_buffer(&rx[index]);
            }
            else
            {
                index = (index + 1) & 1;
                buffer_index = index;
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


static void terminate(int signum)
{
    running = false;
    signal(SIGINT, SIG_DFL);
}


static void parse_opts(int argc, char * argv[])
{
    // here we could change the pixtend model for example
}


static void parse_cmd(union xtendpid_cmds * cmd, size_t cmd_len, union xtendpid_answer * answer, size_t * answer_len)
{
    printf("received:\n");
    print_buffer(&cmd->base.cmd, sizeof(union xtendpid_cmds));

    answer->base.return_code = RC_UNKNOWN_CMD;
    *answer_len = 1;

    printf("sending:\n");
    print_buffer(&answer->base.return_code, *answer_len);
}


int app_main(int argc, char * argv[])
{

    parse_opts(argc, argv);

    printf("initializing pixtend to expect model %c%c... ", model, sub_model);

    if(!pixt_init(&pixt, model, sub_model))
    {
        printf("fail\n");
        return -1;
    }
    else
    {
        printf("pass\n");
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 0x100000);

    int ret = 0;

    mode = 0;
    speed = 700000;
    bits = 8;
    delay = 0;

    printf("initializing SPI... ");

    ret = spi_init();
    if(ret < 0)
    {
         printf("failed\n");
         return ret;
    }
    else
    {
         printf("pass\n");
    }

    signal(SIGINT, terminate);

    printf("creating worker thread... ");

    if(pthread_create(&pth, &attr, worker_thread, NULL) < 0)
    {
        printf("fail\n");
        return -1;
    }
    else
    {
        printf("pass\n");
    }

    printf("initializing zmq... ");

    void * context = zmq_ctx_new();
    void * responder = zmq_socket(context, ZMQ_REP);
    char connect_str[1024];

    snprintf(connect_str, sizeof(connect_str), "tcp://%s:%u", address, port);

    printf("binding to: \"%s\"... ", connect_str);

    int rc = zmq_bind(responder, connect_str);
    if(rc != 0)
    {
        printf("fail\n");
        running = false;
    }
    else
    {
        printf("pass\n");
    }

    printf("accepting messages\n");

    union xtendpid_cmds cmd;
    union xtendpid_answer answer;
    size_t answer_len = 0;

    for(;running;)
    {
        memset(&cmd, 0, sizeof(cmd));
        memset(&answer, 0, sizeof(answer));

        int len = zmq_recv(responder, &cmd, sizeof(cmd), 0);
        if(len < 0)
        {
            printf("error receiving from zmq: %d\n", len);
        }
        else
        {
            // parse the command and do some answering

            if(!running)
            {
                answer.base.return_code = RC_DEAD;
            }
            else
            {
                // do the job
                parse_cmd(&cmd, len, &answer, &answer_len);
            }

            // and send the return code back
            len = zmq_send(responder, &answer.base.return_code, answer_len, 0);
            if(len < 0)
            {
                printf("error sending to zmq: %d\n", len);
            }
        }
    }

    printf("stopping worker thread\n");

    // stop worker thread
    running = false;
    pthread_join(pth, NULL);

    printf("deinit spi\n");

    // close spi
    spi_deinit();

    printf("exit\n");

    return 0;
}
