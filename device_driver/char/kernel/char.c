/*
 *  device char module
 *  
 *  (C) 2021.04.26 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>

static int cmd = 0;
module_param(cmd, int, 0644);
MODULE_PARM_DESC(cmd, "time test commond");

#define DEV_NAME            "char_demo"
#define CHAR_DEMO_MAJOR     (0)
#define CHAR_DEMO_MINOR     (2)


static struct class *char_demo_class;
static struct device *char_demo_dev;
static int char_demo_major;

/* private data */
struct char_demo_pdata{
    int num;
};


/* open */
static int char_demo_open(struct inode *inode, struct file *filp)
{
    struct char_demo_pdata *pdata;

    /* allocate memory to pdata */
    pdata = kzalloc(sizeof(struct char_demo_pdata), GFP_KERNEL);
    if(!pdata)
    {
        printk(KERN_ERR "No free memory!\n");
        return -ENOMEM;
    }
    pdata->num = 0x91;

    /* store on file */
    filp->private_data = pdata;
    
    return 0;
}

/* release */
static int char_demo_release(struct inode *inode, struct file *filp)
{
    struct char_demo_pdata *pdata = (struct char_demo_pdata *)filp->private_data;

    /* safe pointer */
    filp->private_data = NULL;

    /* free private data */
    kfree(pdata);

    return 0;
}

/* read */
static ssize_t char_demo_read(struct file *filp, char __user *buf, size_t len, loff_t *offset)
{
    struct char_demo_pdata *pdata = (struct char_demo_pdata *)filp->private_data;

    if(copy_to_user(buf, pdata, len))
    {
        printk(KERN_ERR "unable copy data to user.\n");
        return -EINVAL;
    }
    return len;
}

/* write */
static ssize_t char_demo_write(struct file *filp, const char __user *buf, size_t len, loff_t *offset)
{
    struct char_demo_pdata *pdata = (struct char_demo_pdata *)filp->private_data;

    if(copy_from_user(pdata, buf, len))
    {
        printk(KERN_ERR "unable to copy from user.\n");
        return -EINVAL;
    }
    printk("data from userland: %d\n", pdata->num);
}


/* file operations */
static struct file_operations char_demo_fops = {
    .owner      = THIS_MODULE,
    .open       = char_demo_open,
    .release    = char_demo_release,
    .write      = char_demo_write,
    .read       = char_demo_read,
};

static __init int char_demo_init(void)
{
    int ret;

    /* register character driver */
    ret =  register_chrdev(CHAR_DEMO_MAJOR, DEV_NAME, &char_demo_fops);
    if(ret < 0)
    {
        printk(KERN_ERR "unable to register character driver.\n");
        ret = -ENODEV;
        goto err_chrdev;
    }
    char_demo_major = ret;

    /* register device into class */
    char_demo_class = class_create(THIS_MODULE, DEV_NAME);
    if(IS_ERR(char_demo_class))
    {
        printk(KERN_ERR "unable to register class subsystem.\n");
        ret = PTR_ERR(char_demo_class);
        goto err_class;
    }

    /* register device into sysfs */
    char_demo_dev = device_create(char_demo_class, NULL, MKDEV(char_demo_major, CHAR_DEMO_MINOR), NULL, DEV_NAME);
    if(IS_ERR(KERN_ERR))
    {
        printk(KERN_ERR "unable to register device subsystem.\n");
        ret = PTR_ERR(char_demo_dev);
        goto err_dev;
    }

    printk(KERN_INFO "%s - major: %d minor: %d\n", DEV_NAME, char_demo_major, CHAR_DEMO_MINOR);
    return 0;

err_dev:
    class_destroy(char_demo_class);
err_class:
    unregister_chrdev(char_demo_major, DEV_NAME);
err_chrdev:

    return ret;
}


/* module exit entry */
static __exit void char_demo_exit(void)
{
    device_destroy(char_demo_class, MKDEV(CHAR_DEMO_MAJOR, CHAR_DEMO_MINOR));
    class_destroy(char_demo_class);
    unregister_chrdev(char_demo_major, DEV_NAME);

    printk("module removed!\n");
}

module_init(char_demo_init);
module_exit(char_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("kernel time operation");

