/*
 *  time
 *  
 *  (C) 2021.03.29 <hkdywg@163.com>
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

static int cmd = 0;
module_param(cmd, int, 0644);
MODULE_PARM_DESC(cmd, "time test commond");

#define TIME_DELAY                  0
#define TIME_GET_REAL_TS64          1
#define TIME_GET_TS64               2

static __init int time_demo_init(void)
{
    switch(cmd)
    {
        case TIME_DELAY:
            printk("Delay Procedure Entence ...\n");
            /*ms 1s=1000ms*/
            mdelay(2000);
            printk("Delay 2000ms done!\n");
            /*ssleep*/
            ssleep(2);
            printk("Sleep 2s done!\n");
        break;
        case TIME_GET_REAL_TS64:
        {
            /* REALTIME 即：墙钟时间，从1970到现在的时间 */
            struct timespec64 tv;
            ktime_get_real_ts64(&tv);
            printk("Current Time: %lld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        break;
        case TIME_GET_TS64:
        {
            /* MONOTONIC 当前时间 */
            struct timespec64 start;
            struct timespec64 end;
            struct timespec64 duration;
            /* Timing start */
            ktime_get_ts64(&start);

            printk("Timing ...\n");
            ssleep(2);
            /* Timing end */
            ktime_get_ts64(&end);

            /* Calculate Timing */
            duration = timespec64_sub(end, start);

            /* output */
            printk("Second:         %lld\n", duration.tv_sec);
            printk("Millisecond:    %ld\n", duration.tv_nsec / NSEC_PER_USEC);
        }
        break;
        default:
            printk("timing: do nothing\n");
        break;
    }

    return 0;
}

static __exit void time_demo_exit(void)
{
    printk("module removed!\n");
}

module_init(time_demo_init);
module_exit(time_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("kernel time operation");

