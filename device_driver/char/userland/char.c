/*
 *  char module driver test application in userland
 *  
 *  (C) 2021.04.27 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* exchange data between kernel and userlan */
struct char_demo_pdata {
    int num;
};


#define DEV_PATH    "/dev/char_demo"

int main(int argc, char *argv[])
{
    struct char_demo_pdata pdata;
    int ret = 0;
    int fd;

    /* open device */
    fd = open(DEV_PATH, O_RDWR);
    if(fd < 0)
    {
        printf("can't open %s\n", DEV_PATH);
        return -1;
    }
    
    do
    {
        /* read information from kernel */
        ret = read(fd, &pdata, sizeof(pdata));
        if(ret != sizeof(pdata))
        {
            printf("bad readding.\n");
            ret = -1;
            break;
        }
        printf("kernel information: %d\n", pdata.num);

        /* write information to kernel */
        pdata.num = 0xaa;
        ret = write(fd, &pdata, sizeof(pdata));
        if(ret != sizeof(pdata))
        {
            printf("bad writing.\n");
            ret = -1;
            break;
        }
        ret = 0;
    }while(0);
    
    close(fd);
    return ret;
}


