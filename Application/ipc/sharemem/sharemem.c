#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>


#define FILE_MODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

struct shmstruct
{
    int count;
};

static sem_t *mutex;
static int count;

int main(int argc, char *argv[])
{
    int fd;
    struct shmstruct *ptr;
    if(argc == 3)
    {
        if((fd = shm_open(argv[1], O_RDWR , FILE_MODE)) == -1)
        {
            perror("shm_open error");
            exit(-1);
        }
        
        ftruncate(fd, sizeof(struct shmstruct));

        ptr = mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if(ptr == MAP_FAILED)
        {
            perror("mmap error");
            exit(-1);
        }
        close(fd);
        if((mutex = sem_open(argv[2], O_RDWR , FILE_MODE, 1)) == SEM_FAILED)
        {
            perror("sem_open error");
            exit(-1);
        }
        sem_wait(mutex);
        printf("pid %ld : %d\n", (long)getpid(), ptr->count);
        sem_post(mutex);
        sem_close(mutex);
    }
    if(argc == 4)
    {
        shm_unlink(argv[1]);
        if((fd = shm_open(argv[1], O_RDWR | O_CREAT | O_EXCL, FILE_MODE)) == -1)
        {
            perror("shm_open error");
            exit(-1);
        }
        
        ftruncate(fd, sizeof(struct shmstruct));

        ptr = mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if(ptr == MAP_FAILED)
        {
            perror("mmap error");
            exit(-1);
        }
        close(fd);
        sem_unlink(argv[2]);
        if((mutex = sem_open(argv[2], O_CREAT | O_EXCL, FILE_MODE, 1)) == SEM_FAILED)
        {
            perror("sem_open error");
            exit(-1);
        }
        sem_wait(mutex);
        ptr->count = atoi(argv[3]);
        printf("write count  %d to shm\n", ptr->count);
        sem_post(mutex);
        sem_close(mutex);
    }
    return 0;
}
