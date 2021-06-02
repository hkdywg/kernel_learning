#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <fcntl.h>

#define SVSHM_MODE  (SHM_R | SHM_W | SHM_R >> 3 | SHM_R >> 6)

int main(int argc, char *argv[])
{
    int c, id, oflag;
    char *ptr;
    size_t length;
    struct shmid_ds buf;
    oflag = SVSHM_MODE | IPC_CREAT;

    while((c = getopt(argc, argv, "csrd")) != -1)
    {
        switch(c)
        {
            case 'c':
            {
                length = atoi(argv[2]);
                id = shmget(ftok(argv[3], 0), length, oflag);
                ptr = shmat(id, NULL, 0);
            }
            break;

            case 's':
            {
                id = shmget(ftok(argv[2], 0), 0, SVSHM_MODE);
                ptr = shmat(id, NULL, 0);
                if(ptr == NULL)
                {
                    printf("get sharemem address error\n");
                    return -1;
                }
                shmctl(id, IPC_STAT, &buf);
                printf("buf.shm_segsz = %d\n", buf.shm_segsz);
                for(int i =0; i < buf.shm_segsz; i++)
                {
                    *ptr++ = i;
                    printf("ptr[%d] = %d\n", i, *ptr);
                }
            }
            break;

            case 'r':
            {
                id = shmget(ftok(argv[2], 0), 0, SVSHM_MODE);
                ptr = shmat(id, NULL, 0);
                shmctl(id, IPC_STAT, &buf);
                for(int i =0; i < buf.shm_segsz; i++)
                {
                    c = *ptr++;
                    printf("ptr[%d] = %d\n", i, c);
                }
            }
            break;
        }
    }
}
