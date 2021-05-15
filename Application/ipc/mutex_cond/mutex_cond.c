#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define MAXSIZE         102400
#define MAXCONSUMER     1000


struct producer_t
{
    pthread_mutex_t mutex;
    int buf[MAXSIZE];
    int nput;
    int nval;
};

struct consumer_t
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int nready;
};

static int nitems;
static struct producer_t producer = {PTHREAD_MUTEX_INITIALIZER};
static struct consumer_t consumer = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};

void *produce(void *);
void *consume(void *);

int main(int argc, char *argv[])
{
    int i, nthreads, count[MAXCONSUMER];
    pthread_t tid_produce[MAXCONSUMER], tid_consume[MAXCONSUMER];
    if(argc != 3)
    {
        printf("usage: mutex_cond  <#itmes> <#threads>.\n");
        exit(0);
    }
    memset(producer.buf, 0, MAXSIZE);
    nitems = atoi(argv[1]);
    nthreads = atoi(argv[2]);
    pthread_setconcurrency(nthreads+1);
    for(i = 0; i < nthreads; i++)
    {
        count[i] = 0;
        pthread_create(&tid_produce[i], NULL, produce, &count[i]);
    }
    for(i = 0; i < nthreads * 40; i++)
    {
        count[i] = 0;
        pthread_create(&tid_consume[i], NULL, consume, &count[i]);
    }
    for(i = 0; i < nthreads; i++)
    {
        pthread_join(tid_produce[i], NULL);
    }
    for(i = 0; i < nthreads * 40; i++)
    {
        pthread_join(tid_consume[i], NULL);
    }
    exit(0);
}


void *produce(void *arg)
{
    usleep(50000);
    sleep(1);
    for( ; ; )
    {
        pthread_mutex_lock(&producer.mutex);
        if(producer.nput >= nitems)
        {
            producer.nput = 0;
            producer.nval = 0;
        }
        producer.buf[producer.nput]= producer.nval;
        producer.nval++;
        producer.nput++;
        pthread_mutex_unlock(&producer.mutex);

        pthread_mutex_lock(&consumer.mutex);
        if(consumer.nready == 0)
        {
            //printf("producer send signal.\n");
            pthread_cond_signal(&consumer.cond);
            //printf("unlock mutex\n");
        }
        consumer.nready++;
        pthread_mutex_unlock(&consumer.mutex);
        *((int *)arg) += 1;
    }
}

static int cnt = 0;

void *consume(void *arg)
{
    //for(i = 0; i < nitems; i++)
    for( ; ; )
    {
        pthread_mutex_lock(&consumer.mutex); 
        //printf("consumer nready = %d\n", consumer.nready);
        while(consumer.nready == 0)
        {
            pthread_cond_wait(&consumer.cond, &consumer.mutex);
            //printf("--------\n");
        }
        consumer.nready--;
        pthread_mutex_unlock(&consumer.mutex);

        pthread_mutex_lock(&producer.mutex);
        cnt++;
        if(cnt >= nitems)
        {
            pthread_mutex_unlock(&producer.mutex);
            return;
        }
        if(producer.buf[cnt] != 0)
        {
            //printf("thread %d print producer buf[%d] = %d\n",pthread_self(), cnt, producer.buf[cnt]);
            printf("buf[%d] = %d\n", cnt, producer.buf[cnt]);
            producer.buf[cnt] = 0;
        }
        pthread_mutex_unlock(&producer.mutex);
    }
    return;
}

