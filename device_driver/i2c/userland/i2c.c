/*
 *  i2c slave test application in userland
 *  
 *  (C) 2021.04.29 <hkdywg@163.com>
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
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2C_BUS         "/dev/i2c-1"
#define I2C_ADDR        (0x50)
#define I2C_M_WR        (0)

static int i2cbus_packread(int fd, unsigned char addr, unsigned char offset,
                           unsigned char *buf, unsigned char len)
{
    unsigned char tmpaddr[2];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset;
    int ret;

    tmpaddr[0] = offset;
    msgs[0].addr = addr & 0xfe;
    msgs[0].flags = I2C_M_WR;
    msgs[0].len = 1;
    msgs[0].buf = tmpaddr;

    tmpaddr[1] = offset;
    msgs[1].addr = addr & 0xfe;
    msgs[1].flags = I2C_M_WR;
    msgs[1].len = 1;
    msgs[1].buf = tmpaddr;

    msgset.msgs = msgs;
    msgset.nmsgs = 2;

    ret = ioctl(fd, I2C_RDWR, &msgset);

    return ret;
}

static int i2cbus_packwrite(int fd, unsigned char addr, unsigned char offset,
                            unsigned char *buf, unsigned char len)
{
    unsigned char tmpaddr[2];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset;
    int ret;

    tmpaddr[0] = offset;
    tmpaddr[1] = buf[0];

    msgs[0].addr = addr & 0xfe;
    msgs[0].flags = I2C_M_WR;
    msgs[0].len = 2;
    msgs[0].buf = tmpaddr;

    msgset.msgs = msgs;
    msgset.nmsgs = 1;

    ret = ioctl(fd, I2C_RDWR, &msgset);

    return ret;
}

int main(int argc, char *argv[])
{
    char msg = 0x89;
    char buf[8];
    int ret = 0;
    int fd;

    /* open i2c bus */
    fd = open(I2C_BUS, O_RDWR);
    if(fd < 0)
    {
        printf("can't open %s\n", I2C_BUS);
        return -1;
    }
    memset(buf, 8, 0);

    /* read operation */
    ret = i2cbus_packread(fd, I2C_ADDR, 0x00, buf, 1);
    if(ret < 0)
    {
        printf("i2c read error\n");
        close(fd);
        return -1;
    }
    printf("DATA: %#x\n", buf[0]);

    close(fd);
    return ret;
}


