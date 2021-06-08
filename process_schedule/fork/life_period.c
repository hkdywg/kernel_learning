#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    pid_t pid, wait_pid;
    int status;

    pid = fork();

    if(pid == 0)
    {
        printf("child process id: %ld\n", getpid());
        pause();
        _exit(0);
    }
    else
    {
        printf("parent process id: %ld\n", getpid());
        wait_pid = waitpid(pid, &status, WUNTRACED | WCONTINUED);
        if(wait_pid == -1)
        {
            perror("connot using waitpid function");
            exit(1);
        }
        if(WIFSIGNALED(status))
            printf("child process is killed by signal %d\n", WTERMSIG(status));

        exit(0);
    }
}
