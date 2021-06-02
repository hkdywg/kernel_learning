#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#define MSG_R   0400    /* read permission */
#define MSG_W   0200    /* write permission */


#define SVMSG_MODE      (MSG_R | MSG_W | MSG_R >> 3 | MSG_R >> 6)

//#define KEY_DEPEND_FILE
#define KEY_MQ          (123456)

struct msgbuf
{
    long msgtype;
    char mtext[256];
};

int main(int argc, char *argv[])
{
    int c, oflag, mqid;
    key_t key;
    struct msgbuf *ptr = NULL;
    size_t len, n;
    long type;
    oflag = SVMSG_MODE;
    
    while((c = getopt(argc, argv, "csr")) != -1)
    {
        switch(c)
        {
            case 'c':
            {
                oflag |= IPC_CREAT;
                /* create key*/
                if((key = ftok(argv[2],0)) == -1)
                {
                    perror("ftok() error");
                    return -1;
                }
                if((mqid = msgget(key, oflag)) == -1)
                {
                    perror("msgget error");
                }
            }
            break;
            case 's':
            {
#ifdef KEY_DEPEND_FILE
                len = atoi(argv[3]);
                type = atol(argv[4]);
                if((key = ftok(argv[2], 0)) == -1)
                {
                    perror("ftok() error");
                    return -1;
                }
                if((mqid = msgget(key, MSG_W)) == -1)
                {
                    printf("msgget msg buf error\p");
                    return -1;
                }
                ptr = malloc(sizeof(long) + len);
                if(ptr == NULL)
                {
                    printf("malloc failed\n");
                    return -1;
                }
                ptr->msgtype = type;
                memcpy(ptr->mtext, argv[5], strlen(argv[5]));
                if(msgsnd(mqid, ptr, len, 0) == -1)
                {
                    perror("msgsnd() error");
                    return -1;
                }
#else
                len = atoi(argv[2]);
                type = atol(argv[3]);
                if((mqid = msgget(KEY_MQ, SVMSG_MODE | IPC_CREAT)) == -1)
                {
                    printf("msgget msg buf error\p");
                    return -1;
                }
                ptr = malloc(sizeof(long) + len);
                if(ptr == NULL)
                {
                    printf("malloc failed\n");
                    return -1;
                }
                ptr->msgtype = type;
                memcpy(ptr->mtext, argv[4], strlen(argv[4]));
                if(msgsnd(mqid, ptr, len, 0) == -1)
                {
                    perror("msgsnd() error");
                    return -1;
                }
#endif
            }
            break;
            case 'r':
            {
                type = 0;
                oflag = 0;
#ifdef KEY_DEPEND_FILE
                len = atoi(argv[3]);
                if((key = ftok(argv[2], 0)) == -1)
                {
                    perror("ftok() error");
                    return -1;
                }
                if((mqid = msgget(key, MSG_R)) == -1)
                {
                    printf("msgget msg buf error\p");
                    return -1;
                }
                ptr = malloc(sizeof(long) + len);
                if(ptr == NULL)
                {
                    printf("malloc failed\n");
                    return -1;
                }
                if((n = msgrcv(mqid, ptr, len, type, oflag)) == -1)
                {
                    perror("msgrcv error\n");
                    return -1;
                }
                else
                    printf("received msg type is %d is %s\n",ptr->msgtype,  ptr->mtext);
#else
                len = atoi(argv[2]);
                if((mqid = msgget(KEY_MQ, MSG_R)) == -1)
                {
                    printf("msgget msg buf error\n");
                    return -1;
                }
                ptr = malloc(sizeof(long) + len);
                if(ptr == NULL)
                {
                    printf("malloc failed\n");
                    return -1;
                }
                if((n = msgrcv(mqid, ptr, len, type, oflag)) == -1)
                {
                    perror("msgrcv error\n");
                    return -1;
                }
                else
                    printf("received msg type is %d is %s\n",ptr->msgtype,  ptr->mtext);
#endif
            }
            break;
            default:
            break;
        }
    }
    if(ptr != NULL)
        free(ptr);
    return 0;
}
