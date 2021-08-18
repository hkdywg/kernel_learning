/*
 *  i2c_bus
 *  
 *  (C) 2021.08.18 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>

#define DEV_NAME "gpio_demo"

struct gpio_demo_priv {
    int irq;
    int gpio;
};

/* Irq Handler */
static irqreturn_t gpio_demo_handler(int irq, void *dev_id)
{
    printk("irq %d handler\n", irq);
    return IRQ_HANDLED;
}

/* Probe: (LDD) Initialize Device */
static int gpio_demo_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    struct gpio_demo_priv *priv;
    int value, gpio, irq;
    int ret;

    priv = kzalloc(sizeof(*priv), GFP_KERNEL);
    if(!priv){
        printk("error: no free memory!!!\n");
        ret = -ENOMEM;
        goto err_alloc;
    }

    gpio = of_get_named_gpio(np, "BD_gpio", 0);
    if(gpio < 0)
    {
        printk("unable to get gpio form dts\n");
        ret = -EINVAL;
        goto err_gpio;
    }

    gpio_direction_output(gpio, 0);

    gpio_set_value(gpio, 1);

    value = gpio_get_value(gpio);

    gpio_direction_input(gpio);

    ret = request_irq(irq, gpio_demo_handler, IRQF_TRIGGER_FALLING, DEV_NAME, NULL);
    if(ret < 0)
    {
        printk("can't request irq %d\n", irq);
        ret = -EINVAL;
        goto err_irq;
    }
    
    priv->gpio = gpio;
    priv->irq = irq;
    platform_set_drvdata(pdev, priv);

    return 0;

err_irq:

err_gpio:
    kfree(priv);
    return ret;

err_alloc:
    return ret;
}

static int gpio_demo_remove(struct platform_device *pdev)
{
    struct gpio_demo_priv *priv = platform_get_drvdata(pdev);

    free_irq(priv->irq, NULL);

    kfree(priv);
    return 0;
}

static const struct of_device_id gpio_demo_of_match[] = {
    { .compatible = "device_driver_test, gpio", },
    {  },
};

MODULE_DEVICE_TABLE(of, gpio_demo_of_match);

/* Platform Driver Information */
static struct platform_driver gpio_demo_driver = {
    .probe      = gpio_demo_probe,
    .remove     = gpio_demo_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = DEV_NAME,
        .of_match_table = gpio_demo_of_match,
    },
};

module_platform_driver(gpio_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg, hkdywg@163.com");
MODULE_DESCRIPTION("gpio device driver");


