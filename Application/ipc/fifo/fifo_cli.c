
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define FIFO1       "/tmp/fifo.1"
#define FIFO2       "/tmp/fifo.2"

int main(int argc, char *argv[])
{
    int readfd, writefd;
    pid_t childpid;
    char buf[100];
    memset(buf, 0, 100);
    if((mkfifo(FIFO1, FILE_MODE) < 0) && (errno != EEXIST))
    {
        perror("mkfifo error");
        exit(-1);
    }
    if((mkfifo(FIFO2, FILE_MODE) < 0) && (errno != EEXIST))
    {
        perror("mkfifo error");
        exit(-1);
    }
    writefd = open(FIFO1, O_WRONLY, 0);
    readfd= open(FIFO2, O_RDONLY, 0);
    read(readfd, buf, 100);
    printf("client received a message from server: %s\n", buf);
    printf("client input a message : ");
    gets(buf);
    write(writefd, buf, strlen(buf));
    close(readfd);
    close(writefd);
    unlink(FIFO1);
    unlink(FIFO2);
    return 0;
}
