#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#define FILE_MODE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

static mqd_t mqd;
static struct mq_attr attr;
static struct sigevent sigev;
static volatile sig_atomic_t mqflag;
static void notify_thread(union sigval);
static void *handle_receivemsg(void *arg);

static void sig_usr1(int signo)
{
    mqflag = 1;
    return;
}

int main(int argc, char *argv[])
{

    int c, flags;
    mqd_t mqd;
    struct mq_attr attr;
    size_t len;
    u_int8_t prio;
    char *ptr, *buf;
    pthread_t pid;
    sigset_t zeromask, newmask, oldmask;

    
    flags = O_RDWR | O_CREAT;
    printf("create mqueue.\n");
    while ((c = getopt(argc, argv, "es")) != -1)
    {
        printf(" c = %d\n", c);
        switch(c)
        {
            case 'e':
                {
                    printf("------\n");
                    if((mqd = mq_open(argv[1], flags, FILE_MODE, NULL)) == -1)
                    {
                        perror("mq_open() error");
                        exit(-1);
                    }
                    mq_getattr(mqd, &attr);
                    printf("msc #msgs = %ld, max #bytes/msg = %ld, #currently on queue = %ld\n", attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
                    
                    pthread_create(&pid, NULL, handle_receivemsg, argv[1]);

                    len = strlen(argv[2]);
                    prio = atoi(argv[3]);
                    ptr = calloc(len, sizeof(char));
                    memcpy(ptr, argv[2], strlen(argv[2]));
                    printf("propare to send msg %s , len is %d\n", ptr, len);
                    if(mq_send(mqd, ptr, len, prio) == -1)
                    {
                        perror("mq_send() error");
                        exit(-1);
                    }
                    printf("mq send success\n");

                    mq_close(mqd);
                    pthread_join(pid, NULL);

                }
                break;
            case 's':
                {
                    if((mqd = mq_open(argv[1], flags, FILE_MODE, NULL)) == -1)
                    {
                        perror("mq_open() error");
                        exit(-1);
                    }
                    mq_getattr(mqd, &attr);
                    buf = malloc(attr.mq_msgsize);
                    sigemptyset(&zeromask);
                    sigemptyset(&newmask);
                    sigemptyset(&oldmask);
                    sigaddset(&newmask, SIGUSR1);
                    signal(SIGUSR1, sig_usr1);
                    sigev.sigev_notify = SIGEV_SIGNAL;
                    sigev.sigev_signo = SIGUSR1;
                    if(mq_notify(mqd, &sigev) == -1)
                    {
                        perror("mq_notify error");
                        exit(-1);
                    }
                    for( ; ; )
                    {
                        sigprocmask(SIG_BLOCK, &newmask, &oldmask);
                        while(mqflag == 0)
                            sigsuspend(&zeromask);
                        mqflag = 0;
                        mq_notify(mqd, &sigev);
                        mq_receive(mqd, buf, attr.mq_msgsize, NULL);
                        printf("receive msg %s\n", buf);
                        sigprocmask(SIG_UNBLOCK, &newmask, NULL);
                    }
                }
                break;
        }
    }
    
    return 0;
}


static void notify_thread(union sigval arg)
{
    ssize_t n;
    void *buf;
    printf("notify_thread started\n");
    buf = malloc(attr.mq_msgsize);
    mq_notify(mqd, &sigev);
    while(n = mq_receive(mqd, buf, attr.mq_msgsize, NULL) >= 0)
    {
        perror("mq_receive error");
        exit(-1);
    }
    free(buf);
    pthread_exit(NULL);
}

static void *handle_receivemsg(void *arg)
{
    mqd_t mqd;
    ssize_t n;
    u_int8_t prio;
    char *buf;
    struct mq_attr attr;
    sleep(1);
    printf("arg is %s\n", arg);
    mqd = mq_open(arg, O_RDONLY | O_NONBLOCK);
    if(mqd == -1)
    {
        perror("mq_open faild");
        return -1;
    }
    mq_getattr(mqd, &attr);
    buf = malloc(attr.mq_msgsize);
    printf("mqd mq_msgsize is %d\n", attr.mq_msgsize);
    if((n = mq_receive(mqd, buf, attr.mq_msgsize, &prio)) == -1)
    {
        perror("mq_receive error: ");
        exit(-1);
    }
    printf("read msg is %s %ld bytes, priority = %u\n", buf, (long)n, prio);
    return 0;
}
