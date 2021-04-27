/*
 *  platform
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
#include <linux/interrupt.h>
#include <linux/io.h>

#define DEV_NAME            "platform_resource"

/* platform device data */
struct platform_demo_data {
    unsigned long index;
};

/* platform device data */
struct platform_demo_pdata {
    struct device *dev;
    void __iomem *base;
    unsigned int irq;
};

/* IRQ handler */
static irqreturn_t platform_demo_irqhandler(int irq, void *param)
{
    return IRQ_HANDLED;
}

/* probe: initialize device */
static int platform_demo_probe(struct platform_device *pdev)
{
    struct platform_demo_pdata *private_data;
    struct platform_demo_data *device_data;
    struct resource *mem_res;
    struct resource *irq_res;
    void __iomem *mem_base;
    int ret;

    /* platform private data */
    device_data = pdev->dev.platform_data;
    if(!device_data)
    {
        dev_err(&pdev->dev, "unable to get drvdata!\n");
        return -EINVAL;
    }
    printk("device paltform_data: %#lx\n", device_data->index);

    /* private platform: allocate memory */
    private_data = devm_kzalloc(&pdev->dev, sizeof(*private_data), GFP_KERNEL);
    if(!private_data)
    {
        dev_err(&pdev->dev, "private data no free memory.\n");
        ret = -ENOMEM;
        goto pdata_mem;
    }

    /* first memory region */
    mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(!mem_res)
    {
        dev_err(&pdev->dev, "IORESOURCE_MEM 0 unavalible");
        ret = -ENODEV;
        goto mem_0;
    }
    printk("resource: %s\n", mem_res->name);
    printk("region: %#lx - %#lx\n", (unsigned long)mem_res->start, (unsigned long)mem_res->end);

    /* second memory region */
    mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if(!mem_res)
    {
        dev_err(&pdev->dev, "IORESOURCE_MEM 1 unavalible");
        ret = -ENODEV;
        goto mem_0;
    }
    printk("resource: %s\n", mem_res->name);
    printk("region: %#lx - %#lx\n", (unsigned long)mem_res->start, (unsigned long)mem_res->end);

    /* request memory region */
    if(!devm_request_mem_region(&pdev->dev, mem_res->start, resource_size(mem_res), mem_res->name))
    {
        printk("unmap memory region: %#lx - %#lx\n", (unsigned long)mem_res->start, (unsigned long)mem_res->end);
        ret = -EBUSY;
        goto mem_req;
    }

    /* remap memory region */
    mem_base = devm_ioremap(&pdev->dev, mem_res->start, resource_size(mem_res));
    if(IS_ERR(mem_base))
    {
        printk(KERN_ERR "unmap memory region: %#lx - %#lx\n", (unsigned long)mem_res->start, (unsigned long)mem_res->end);
        ret = PTR_ERR(mem_base);
        goto mem_remap;
    }

    /* store on private data */
    private_data->base = mem_base;
    /* memory region ok */
    *(unsigned long *)mem_base = 0x902989;
    printk("Vaddr: %#lx value: %#lx\n", (unsigned long)mem_base, *(unsigned long *)mem_base);

    /* first irq */
    irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if(!irq_res)
    {
        dev_err(&pdev->dev, "IRQRESOURCE unavailable.\n");
        ret = -ENODEV;
        goto irq_0;
    }
    /* second irq */
    irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 1);
    if(!irq_res)
    {
        dev_err(&pdev->dev, "IRQRESOURCE unavailable.\n");
        ret = -ENODEV;
        goto irq_1;
    }

    /* request irq resource */
    ret = devm_request_irq(&pdev->dev, irq_res->start, platform_demo_irqhandler, 0, "platform", NULL);
    if(ret)
    {
        dev_err(&pdev->dev, "unable to request IRQ.\n");
        goto irq_req;
    }

    /* store on private data */
    private_data->irq = irq_res->start;
    printk("register irq: %d handler: platform_demo_irqhandler()\n", irq_res->start);

    /* bind private data into platform device */
    private_data->dev = &pdev->dev;
    platform_set_drvdata(pdev, private_data);

   return 0;

irq_req:
irq_1:
irq_0:
   devm_iounmap(&pdev->dev, mem_base);
mem_remap:
mem_req:
mem_1:
mem_0:
   devm_kfree(&pdev->dev, private_data);
pdata_mem:
   return ret;
}

/* remove: remove device (module) */
static int platform_demo_remove(struct platform_device *pdev)
{
    struct platform_demo_pdata *pdata = platform_get_drvdata(pdev);

    /* remove irq */
    devm_free_irq(&pdev->dev, pdata->irq, NULL);

    /* remove ioremap */
    devm_iounmap(&pdev->dev, pdata->base);

    /* remove private data */
    platform_set_drvdata(pdev, NULL);
    devm_kfree(&pdev->dev, pdata);

    return 0;
}

/* shutdown: power off/shutdown */
static void platform_demo_shutdown(struct platform_device *pdev)
{

}

/* suspend: suspend (schedule) sleep */
static int platform_demo_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}


/* resume: (schedule) from suspend/sleep */
static int platform_demo_resume(struct  platform_device *pdev)
{
    return 0;
}

/* release */
static void platform_demo_dev_release(struct device *dev)
{
    dev->parent = NULL;
}

/* platform driver information */
static struct platform_driver platform_demo_driver = {
    .probe      = platform_demo_probe,
    .remove     = platform_demo_remove,
    .shutdown   = platform_demo_shutdown,
    .suspend    = platform_demo_suspend,
    .resume     = platform_demo_resume,
    .driver     = {
        .owner = THIS_MODULE,
        .name  = DEV_NAME,
    },
};

/* platform resource */
static struct resource platform_demo_resources[] = {
    [0] = {     /* memory region 0 */
        .name   = "platform memory 0",
        .start  = 0x22002000,
        .end    = 0x22002008,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {     /* memory region 0 */
        .name   = "platform memory 1",
        .start  = 0x22003000,
        .end    = 0x22003008,
        .flags  = IORESOURCE_MEM,
    },
    [2] = {     /* memory region 0 */
        .name   = "platform IRQ 0",
        .start  = 12,
        .end    = 12,
        .flags  = IORESOURCE_IRQ,
    },
    [3] = {     /* memory region 0 */
        .name   = "platform IRQ 1",
        .start  = 38,
        .end    = 38,
        .flags  = IORESOURCE_IRQ,
    },
};

/* platform device private data */
static struct platform_demo_data platform_demo_pdata = {
    .index = 0x10,
};


static struct platform_device platform_demo_device = {
    .name = DEV_NAME,
    .id =1,
    .dev = {
        .platform_data = &platform_demo_pdata,
        .release = platform_demo_dev_release,
    },
    .resource =  platform_demo_resources,
    .num_resources = ARRAY_SIZE(platform_demo_resources),
};


/* module initialize entry */
static __init int platform_demo_init(void)
{
    int ret;
    
    ret = platform_driver_register(&platform_demo_driver);
    if(ret)
    {
        printk("Error: platform driver register.\n");
        return -EBUSY;
    }

    ret = platform_device_register(&platform_demo_device);
    if(ret)
    {
        printk("Error: platform device register.\n");
        return -EBUSY;
    }
    return 0;
}


/* module exit entry */
static __exit void platform_demo_exit(void)
{
    platform_device_unregister(&platform_demo_device);
    platform_driver_unregister(&platform_demo_driver);
    printk("module removed!\n");
}

module_init(platform_demo_init);
module_exit(platform_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("kernel time operation");

