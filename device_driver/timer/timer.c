/*
 *  timer
 *  
 *  (C) 2021.04.27 <hkdywg@163.com>
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
#include <linux/platform_device.h>



#define DEV_NAME            "timer_demo"

/* timer */
static struct timer_list timer;

/* timer period */
#define TIMER_DEMO_PERIOD       (1000)  /* 1000ms */

/* timer: handler timeout */
static void timer_demo_handler(struct timer_list *unused)
{
    printk("timer timeout...\n");
    /* timer: setup timerout */
    timer.expires = jiffies + msecs_to_jiffies(TIMER_DEMO_PERIOD);
    /*timer: register */
    add_timer(&timer);
}

/* probe: initialize device */
static int timer_demo_probe(struct platform_device *pdev)
{
    /* timer: setup timer */
    timer_setup(&timer, timer_demo_handler, 0);
    /* timer: setup timeout */
    timer.expires = jiffies + msecs_to_jiffies(TIMER_DEMO_PERIOD);
    /* timer: register */
    add_timer(&timer);

   return 0;
}

/* remove: remove device (module) */
static int timer_demo_remove(struct platform_device *pdev)
{
    /* timer: unregister */
    del_timer(&timer);

    return 0;
}

/* shutdown: power off/shutdown */
static void timer_demo_shutdown(struct platform_device *pdev)
{

}

/* suspend: suspend (schedule) sleep */
static int timer_demo_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}


/* resume: (schedule) from suspend/sleep */
static int timer_demo_resume(struct  platform_device *pdev)
{
    return 0;
}

/* release */
static void timer_demo_dev_release(struct device *dev)
{
    dev->parent = NULL;
}

/* platform driver information */
static struct platform_driver timer_demo_driver = {
    .probe      = timer_demo_probe,
    .remove     = timer_demo_remove,
    .shutdown   = timer_demo_shutdown,
    .suspend    = timer_demo_suspend,
    .resume     = timer_demo_resume,
    .driver     = {
        .owner = THIS_MODULE,
        .name  = DEV_NAME,
    },
};

static struct platform_device timer_demo_device = {
    .name = DEV_NAME,
    .id =1,
    .dev = {
        .release = timer_demo_dev_release,
    },
};

/* module initialize entry */
static __init int timer_demo_init(void)
{
    int ret;
    
    ret = platform_driver_register(&timer_demo_driver);
    if(ret)
    {
        printk("Error: platform driver register.\n");
        return -EBUSY;
    }

    ret = platform_device_register(&timer_demo_device);
    if(ret)
    {
        printk("Error: platform device register.\n");
        return -EBUSY;
    }
    return 0;
}


/* module exit entry */
static __exit void timer_demo_exit(void)
{
    platform_device_unregister(&timer_demo_device);
    platform_driver_unregister(&timer_demo_driver);
    printk("module removed!\n");
}

module_init(timer_demo_init);
module_exit(timer_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("kernel time operation");

