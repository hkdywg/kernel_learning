/*
 *  mdio module
 *  
 *  (C) 2021.11.10 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <linux/err.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/bitops.h>
#include <linux/phy.h>

#define DEV_NAME "mdio_demo"

#define MDIO_READ   (0x00)
#define MDIO_WRITE  (0x01)

/* mdio_demo from real ethernet, and export it here! */
extern struct mii_bus *mdio_demo;

/* parse input string
 * Value[0]: DeviceAddress. Value[1]: register, Value[2]: data 
 * Read/Write operation
 * CMD: <r/w>, <devaddr>, <regaddr>, <value>
 */

static int parse_input_string(const char *string, int *value, int *flag)
{
    int nr;
    char *buffer, *tmp, *leg;
    char tmp_data[20], data;
    int i = 0;

    buffer = (char *) kmalloc(strlen(string) + 1, GFP_KERNEL);
    leg = buffer;
    memset(buffer, 0, strlen(string) + 1);
    strcpy(buffer, string);

    while((tmp = strstr(buffer,",")))
    {
        nr = tmp - buffer;
        tmp++;
        strncpy(tmp_data, buffer, nr);
        tmp_data[nr] = '\0';
        if(strcmp(tmp_data, 'r') == 0)
        {
           *flag = MDIO_READ; 
        }
        else if(strcmp(tmp_data, 'w') == 0)
        {
            *flag = MDIO_WRITE;
        }
        else
        {
            sscanf(tmp_data, "%d", &data);
            value[i++] = data;
        }
        buffer = tmp;
    }
    kfree(leg);
    return 0;
}

static ssize_t mdio_demo_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return 0;
}

static ssize_t mdio_demo_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int operation = MDIO_READ;
    int value[10];


    parse_input_string(buf, value, &operation);
    if(operation == MDIO_READ)
    {
        unsigned short reg;
        reg = mdio_demo->read(mdio_demo, value[0], value[1]);
        printk("\r\n Read: Dev[%x] Reg[%x] Value[%x]\n", value[0], value[1], reg);
    }
    else if(operation == MDIO_WRITE)
    {
        mdio_demo->write(mdio_demo, value[0], value[1], value[2]);
        printk("\r\n Write: Dev[%x] Reg[%x] Value[%x]\n", value[0], value[1], value[2]);
    }
    else
    {
        printk(KERN_ERR "unknown operation!!!\n");
    }
    return size;
}

static struct device_attribute mdio_demo_attr = __ATTR_RW(mdio_demo);

static int mdio_demo_probe(struct platform_device *pdev)
{
    int err;

    err = device_create_file(&pdev->dev, &mdio_demo_attr);
    if(err)
    {
        printk("unable to create device \n");
        return -EINVAL;
    }
    return 0;
}

static int mdio_demo_remove(struct platform_device *pdev)
{
    device_remove_file(&pdev->dev, &mdio_demo_attr);

    return 0;
}


static struct platform_device mdio_demo_device = {
    .name = DEV_NAME,
    .id = -1,
};

static struct platform_driver mdio_demo_driver = {
    .probe = mdio_demo_probe,
    .remove = mdio_demo_remove,
    .driver = {
        .name  = DEV_NAME,
    },
};


module_platform_driver(mdio_demo_driver);

