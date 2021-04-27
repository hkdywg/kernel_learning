/*
 *  platform device module driver test application in userland
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
#include <dirent.h>

#define PLATFORM_PATH   "/sys/bus/platform/devices/platform_resource/"

int main(int argc, char *argv[])
{
    int ret;
    DIR *dir;
    struct dirent *ptr;

    if((dir = opendir(PLATFORM_PATH)) == NULL)
    {
        printf("open %s dirent error.\n", PLATFORM_PATH);
        return -1;
    }

    while((ptr = readdir(dir)) != NULL)
    {
        if((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
            continue;

        switch(ptr->d_type)
        {
            case DT_REG:
                  printf("[FILE] %s\n", ptr->d_name);
            break;
            case DT_DIR:
                  printf("[DIRT] %s\n", ptr->d_name);
            break;
            case DT_FIFO:
                  printf("[FIFO] %s\n", ptr->d_name);
            break;
            case DT_SOCK:
                  printf("[SOCK] %s\n", ptr->d_name);
            break;
            case DT_CHR:
                  printf("[CHR] %s\n", ptr->d_name);
            break;
            case DT_BLK:
                  printf("[BLK] %s\n", ptr->d_name);
            break;
            case DT_LNK:
                  printf("[LINK] %s\n", ptr->d_name);
            break;
        }
    }
    
    close(dir);

    return ret;
}


