#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define MAXDATA         1024
#define MAXREADER       100
#define MAXWRITER       100

struct rw_res
{
    pthread_rwlock_t rwlock;
    char data[MAXDATA];
};

struct rw_res rw_res_t = {PTHREAD_RWLOCK_INITIALIZER};

void *reader(void *arg);
void *writer(void *arg);

int main(int argc, char *argv[])
{
    int i, readercount, writercount;
    pthread_t tid_reader[MAXREADER], tid_writer[MAXWRITER];
    if(argc != 3)
    {
        printf("usage : <rwlock> #<readercount> #<writercount>\n");
        exit(0);
    }

    readercount = atoi(argv[1]);
    writercount = atoi(argv[2]);

    pthread_setconcurrency(readercount + writercount);

    for(i = 0; i < writercount; i++)
        pthread_create(&tid_writer[i], NULL, writer, NULL);
    sleep(1);
    for(i = 0; i < readercount; i++)
        pthread_create(&tid_reader[i], NULL, reader, NULL);

    for(i = 0; i < writercount; i++)
        pthread_join(tid_writer[i], NULL);
    for(i = 0; i < readercount; i++)
        pthread_join(tid_reader[i], NULL);

    return 0;
}


void *reader(void *arg)
{
    pthread_rwlock_rdlock(&rw_res_t.rwlock);
    printf("Reader begins read message.\n");
    printf("Read message is : %s\n", rw_res_t.data);
    pthread_rwlock_unlock(&rw_res_t.rwlock);
    return NULL;
}

void *writer(void *arg)
{
    char data[MAXDATA];
    pthread_rwlock_wrlock(&rw_res_t.rwlock);
    printf("Writers begin write message.\n");
    printf("Enter the write message: \n");
    gets(data);
    strcat(rw_res_t.data, data);
    pthread_rwlock_unlock(&rw_res_t.rwlock);
    return NULL;
}
