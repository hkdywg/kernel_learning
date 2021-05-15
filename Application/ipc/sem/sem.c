#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#define FILE_MODE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define NBUFF           10
#define SEM_MUTEX       "mutex1"
#define SEM_NEMPTY      "nempty1"
#define SEM_NSTORED     "nstored1"

int nitems;

struct shared
{
    int buf[NBUFF];
    sem_t *mutex,*nempty,*nstored;
};

struct shared shared_t;

char *px_ipc_name(const char *name);
void *produce(void *arg);
void *consume(void *arg);

int main(int argc, char *argv[])
{
    pthread_t tid_produce, tid_consume;
    if(argc != 2)
    {
        printf("usage : prodcons <#itmes>\n");
        exit(0);
    }
    nitems = atoi(argv[1]);

    shared_t.mutex = sem_open(SEM_MUTEX, O_CREAT, FILE_MODE, 1);
    if(shared_t.mutex ==  SEM_FAILED)
    {
        perror("sem_open error\n");
        exit(-1);
    }

    shared_t.nempty = sem_open(SEM_NEMPTY, O_CREAT, FILE_MODE, 1);
    if(shared_t.nempty ==  SEM_FAILED)
    {
        perror("sem_open error\n");
        exit(-1);
    }
    
    shared_t.nstored = sem_open(SEM_NSTORED, O_CREAT, FILE_MODE, 1);
    if(shared_t.nstored ==  SEM_FAILED)
    {
        perror("sem_open error\n");
        exit(-1);
    }

    pthread_setconcurrency(2);

    pthread_create(&tid_produce, NULL, produce, NULL);
    pthread_create(&tid_consume, NULL, consume, NULL);

    pthread_join(tid_produce, NULL);
    pthread_join(tid_consume, NULL);

    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_NEMPTY);
    sem_unlink(SEM_NSTORED);

    return 0;
}


void *produce(void *arg)
{
    int i;
    printf("produce is called\n");

    for(i = 0; i < nitems; i++)
    {
        sem_wait(shared_t.nempty);
        sem_wait(shared_t.mutex);
        printf("produce a new item.\n");
        shared_t.buf[i%NBUFF] = i;
        sem_post(shared_t.mutex);
        sem_post(shared_t.nstored);
    }
    return NULL;
}

void *consume(void *arg)
{
    int i;
    printf("consume is called.\n");
    for(i = 0; i < nitems; i++)
    {
        sem_wait(shared_t.nstored);
        sem_wait(shared_t.mutex);
        printf("buff[%d] = %d\n", i, shared_t.buf[i%NBUFF]);
        sem_post(shared_t.mutex);
        sem_post(shared_t.nempty);
    }
    return NULL;
}
