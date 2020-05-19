#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

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

static const struct pixtend * pixt;
static union pixtOut tx;        // no need for double buffer here
static union pixtIn rx[2];      // only worker thread should write this!
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


static bool cmd_get_model(union xtendpid_cmds * cmd, size_t cmd_len, union xtendpid_answer * answer, size_t * answer_len)
{
    if(cmd_len == sizeof(struct xtendpid_cmd_get_model)) {
        size_t rd_index = (buffer_index + 1) & 1;

        char model = '?';
        char sub_model = '?';

        if(pixt_get_model(pixt, &rx[rd_index], &model, &sub_model))
        {
            answer->base.return_code = RC_SUCCESS;
            answer->get_model.model = model;
            answer->get_model.sub_model = sub_model;

            *answer_len = sizeof(answer->get_model);
            return true;
        }
    }
    return false;
}

static bool cmd_get_fw_version(union xtendpid_cmds * cmd, size_t cmd_len, union xtendpid_answer * answer, size_t * answer_len)
{
    if(cmd_len == sizeof(struct xtendpid_cmd_get_fw_version)) {
        size_t rd_index = (buffer_index + 1) & 1;

        uint8_t version = 0;
        if(pixt_get_fw_version(pixt, &rx[rd_index], &version))
        {
            answer->base.return_code = RC_SUCCESS;
            answer->get_fw_version.version = version;

            *answer_len = sizeof(answer->get_fw_version);

            return true;
        }
    }
    return false;
}

static bool cmd_get_hw_version(union xtendpid_cmds * cmd, size_t cmd_len, union xtendpid_answer * answer, size_t * answer_len)
{
    if(cmd_len == sizeof(struct xtendpid_cmd_get_hw_version)) {
        size_t rd_index = (buffer_index + 1) & 1;

        uint8_t version = 0;
        if(pixt_get_hw_version(pixt, &rx[rd_index], &version))
        {
            answer->base.return_code = RC_SUCCESS;
            answer->get_hw_version.version = version;

            *answer_len = sizeof(answer->get_hw_version);

            return true;
        }
    }
    return false;
}


static void prepare_answer_get_di(union xtendpid_answer * answer, size_t * answer_len, size_t di, uint8_t value)
{
    answer->base.return_code = RC_SUCCESS;
    answer->get_di.di = di;
    answer->get_di.value = value;
    *answer_len = sizeof(answer->get_di);
}


static bool cmd_get_di(union xtendpid_cmds * cmd, size_t cmd_len, union xtendpid_answer * answer, size_t * answer_len)
{
    if(cmd_len == sizeof(struct xtendpid_cmd_get_di)) {

        size_t rd_index = (buffer_index + 1) & 1;

        uint8_t value = pixt_get_di(pixt, &rx[rd_index], cmd->get_di.di);
        if(value != 0xff) {
            prepare_answer_get_di(answer, answer_len, cmd->get_di.di, value);
            return true;
        }
    }
    return false;
}


static const bool (*command_handlers[])(union xtendpid_cmds *, size_t, union xtendpid_answer *, size_t *) = {
/* 0 */ cmd_get_model,
/* 1 */ cmd_get_fw_version,
/* 2 */ cmd_get_hw_version,
/* 3 */ cmd_get_di,
};


static void parse_cmd(union xtendpid_cmds * cmd, size_t cmd_len, union xtendpid_answer * answer, size_t * answer_len)
{
    printf("received:\n");
    print_buffer(&cmd->base.cmd, sizeof(union xtendpid_cmds));

    answer->base.cmd = cmd->base.cmd;
    answer->base.return_code = RC_UNKNOWN_CMD;
    *answer_len = 1;

    if(cmd->base.cmd < sizeof(command_handlers) / sizeof(void*))
    {
        pthread_mutex_lock(&mutex);

        if(!command_handlers[cmd->base.cmd](cmd, cmd_len, answer, answer_len))
        {
            answer->base.return_code = RC_FAIL;
        }

        pthread_mutex_unlock(&mutex);
    }

    printf("sending:\n");
    print_buffer(&answer->base.return_code, *answer_len);
}


static void * worker_thread(void * user_data)
{
    int ret = 0;

    const size_t transfer_size = pixt_get_transfer_size(pixt);
    const size_t di_num = pixt_get_num_di(pixt);

    void * context = user_data;
    void * publisher = zmq_socket(context, ZMQ_PUB);
    char connect_str[1024];

    snprintf(connect_str, sizeof(connect_str), "tcp://%s:%u", address, port + 1);

    ret = zmq_bind(publisher, connect_str);
    if(ret != 0)
    {
        printf("binding publisher port failed!/n");
        running = false;
    }

    size_t cnt = 0;
    size_t di;

    union xtendpid_answer answer;
    size_t answer_len;

    for(;running;++cnt)
    {
        pthread_mutex_lock(&mutex);

        int index = buffer_index;
        int new_index = index;              // new data will go here
        int old_index = (index + 1) & 1;    // afterwards, this points to the old data until it gets updated

        pixt_prepare_output(pixt, &tx);

        // here we can do the transfer
        struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)&tx,
                .rx_buf = (unsigned long)&rx[new_index],
                .len = transfer_size,
                .delay_usecs = delay,
                .speed_hz = speed,
                .bits_per_word = bits,
        };

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if(ret > 0)
        {
            if(!pixt_parse_input(pixt, &rx[new_index]))
            {
                // do we really want to exit if we got a crc error?
                ret = -1;
                // print_buffer(&rx[index]);
            }
            else
            {
                buffer_index = old_index;
            }
        } else {
            printf("SPI transfer error!\n");
        }

        pthread_mutex_unlock(&mutex);

        if(ret <= 0) break;

        // check digital inputs
        for(di = 0; di < di_num; ++di)
        {
            uint8_t new_value;
            if((new_value = pixt_get_di(pixt, &rx[new_index], di)) != pixt_get_di(pixt, &rx[old_index], di))
            {
                prepare_answer_get_di(&answer, &answer_len, di, new_value);

                // ship it without blocking
                zmq_send(publisher, &answer, answer_len, 0);
            }
        }

        // here we could take care of all the other stuff like temperature and humidity

        // do it every 100 ms
        usleep(100000);
    }

    if(running)
    {
        printf("unexpected end of worker thread! ret is: %d\n", ret);
        running = false;
    }

    zmq_close(publisher);

    return NULL;
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


int app_main(int argc, char * argv[])
{

    parse_opts(argc, argv);

    printf("initializing pixtend to expect model %c%c... ", model, sub_model);

    pixt = pixt_get(model, sub_model);
    if(!pixt)
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

    printf("creating worker thread... ");

    if(pthread_create(&pth, &attr, worker_thread, context) < 0)
    {
        printf("fail\n");
        return -1;
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

    // close sockets and context
    printf("cleanup sockets\n");
    zmq_close(responder);
    zmq_ctx_destroy(context);

    printf("exit\n");

    return 0;
}
