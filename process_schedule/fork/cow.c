#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int data = 10;

int child_process()
{
    printf("child process %d, data %d\n", getpid(), data);
    data = 20;
    printf("child process %d, data %d\n", getpid(), data);
    _exit(0);
}


int main(int argc, char *argv[])
{
    int pid;
    char opt;

    while((opt = getopt(argc, argv, "f::v")) != -1)
    {
        switch(opt)
        {
            case 'f':
                pid = fork();
            break;
            case 'v':
                pid = vfork();
            break;
            default:
            break;
        }
    }


    if(pid == 0)
    {
        child_process();
    }
    else
    {
        sleep(1);
        printf("Parent process %d, data %d\n", getpid(), data);
        exit(0);
    }
}
