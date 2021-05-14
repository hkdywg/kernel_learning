#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int fd1[2], fd2[2];
    pid_t childpid;
    char buf[100];

    memset(buf, 0, 100);

    if(pipe(fd1) == -1)
    {
        perror("pipe() error");
        exit(-1);
    }
    if(pipe(fd2) == -1)
    {
        perror("pipe() error");
        exit(-1);
    }
    childpid = fork();
    if(childpid == 0)
    {
        printf("child input a message : ");
        gets(buf);
        close(fd1[0]);
        close(fd2[1]);
        write(fd1[1], buf, strlen(buf));
        read(fd2[0], buf, 100);
        printf("child received message from parent : %s\n", buf);
        exit(0);
    }
    if(childpid == -1)
    {
        perror("fork error");
        exit(-1);
    }
    close(fd1[1]);
    close(fd2[0]);
    read(fd1[0], buf, 100);
    printf("parent received message form child : %s\n", buf);
    printf("parent input message: ");
    gets(buf);
    write(fd2[1], buf, strlen(buf));
    waitpid(childpid, NULL, 0);
    return 0;
}
