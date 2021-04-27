/*
 *  device workqueue
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



#define DEV_NAME            "workqueue_demo"

#define INPUT_PERIOD        (1000)

struct wq_demo_pdata {
    struct work_struct wq;
    struct timer_list timer;
};

/* work queue handler */
void wq_isr(struct work_struct *work)
{
    printk("work...\n");
}

/* timer interrupt handler */
static void timer_isr(struct timer_list *unused)
{
    struct wq_demo_pdata *pdata;

    pdata = container_of(unused, struct wq_demo_pdata, timer);

    /* wakeup interrupt bottom */
    schedule_work(&pdata->wq);

    /* timer: setup timeout */
    pdata->timer.expires = jiffies + msecs_to_jiffies(INPUT_PERIOD);

    /* timer register */
    add_timer(&pdata->timer);
}

/* probe: initialize device */
static int wq_demo_probe(struct platform_device *pdev)
{
    struct wq_demo_pdata *pdata;
    int ret;

    pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
    if(!pdata)
    {
        printk("Error: alloc pdata\n");
        ret = -ENOMEM;
        goto err_alloc;
    }

    /* initialize wq */
    INIT_WORK(&pdata->wq, wq_isr);

    /* Emulate interrupt via timeer */
    timer_setup(&pdata->timer, timer_isr, 0);
    pdata->timer.expires = jiffies + msecs_to_jiffies(100);
    add_timer(&pdata->timer);

    platform_set_drvdata(pdev, pdata);

    return 0;
err_alloc:
    return ret;
}

/* remove: remove device (module) */
static int wq_demo_remove(struct platform_device *pdev)
{
    struct wq_demo_pdata *pdata = platform_get_drvdata(pdev);

    del_timer(&pdata->timer);
    kfree(pdata);

    return 0;
}

/* platform driver information */
static struct platform_driver wq_demo_driver = {
    .probe      = wq_demo_probe,
    .remove     = wq_demo_remove,
    .driver     = {
        .owner = THIS_MODULE,
        .name  = DEV_NAME,
    },
};

static struct platform_device *pdev;

/* module initialize entry */
static __init int wq_demo_init(void)
{
    int ret;
    
    ret = platform_driver_register(&wq_demo_driver);
    if(ret)
    {
        printk("Error: platform driver register.\n");
        return -EBUSY;
    }

    pdev = platform_device_register_simple(DEV_NAME, 1, NULL, 0);
    if(IS_ERR(pdev))
    {
        printk("Error: platform device register.\n");
        return PTR_ERR(pdev);
    }
    return 0;
}


/* module exit entry */
static __exit void wq_demo_exit(void)
{

    platform_device_unregister(pdev);
    platform_driver_unregister(&wq_demo_driver);
    printk("module removed!\n");
}

module_init(wq_demo_init);
module_exit(wq_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("kernel time operation");

