/*
 *  led
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

static int leds_drv_init(void)
{

}

static void leds_drv_exit(void)
{

}

module_init(leds_drv_init);
module_exit(leds_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg, hkdywg@163.com");
MODULE_DESCRIPTION("gpio device driver");


