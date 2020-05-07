#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "app_interface.h"


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t pth;
static volatile bool running = true;


static void * worker_thread(void * user_data)
{

    int i = 0;

    for(;running;++i)
    {
        pthread_mutex_lock(&mutex);
        printf("hello from worker %d\n", i);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
    return NULL;
}


int app_main(int argc, char * argv[])
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    pthread_attr_setstacksize(&attr, 0x100000);

    // pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&pth, &attr, worker_thread, NULL) < 0)
    {
        printf("worker_thread pthread_create failed");
        return -1;
    }


    int i;

    for(i=0; i<10; ++i)
    {
        pthread_mutex_lock(&mutex);
        printf("hello from mainloop %d\n", i);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }

    running = false;
    pthread_join(pth, NULL);

    return 0;
}
