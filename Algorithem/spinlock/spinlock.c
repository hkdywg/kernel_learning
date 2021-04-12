/*
 *  spinlock
 *  
 *  (C) 2021.04.12 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>

static int cmd = 0;
module_param(cmd, int, 0644);
MODULE_PARM_DESC(cmd, "spinlock test commond");

#define SPIN_LOCK_SAMPLE         (0x00)
#define SPIN_LOCK_IRQ            (0x01)
#define SPIN_LOCK_TRY            (0x02)

static spinlock_t test_spinlock;

static __init int spinlock_demo_init(void)
{
    switch(cmd)
    {
        case SPIN_LOCK_SAMPLE:
            /* Initialize spinlock */
            spin_lock_init(&test_spinlock);

            /* acquire spinlock */
            spin_lock(&test_spinlock);

            __asm__ volatile ("nop");

            /* release lock */
            spin_unlock(&test_spinlock);
            
            printk("sample spin lock done\n");
        break;
        case SPIN_LOCK_IRQ:
            /* Initialize spinlock */
            spin_lock_init(&test_spinlock);

            /* acquire spinlock and disable local CPU interrupt */
            spin_lock_irq(&test_spinlock);

            __asm__ volatile ("nop");

            /* release lock */
            spin_unlock_irq(&test_spinlock);
            
            printk("irq spin lock done\n");
        break;
        case SPIN_LOCK_TRY:
            /* Initialize spinlock */
            spin_lock_init(&test_spinlock);

            /* acquire spinlock */
            if(spin_trylock(&test_spinlock))
            {
                __asm__ volatile ("nop");
                /* release lock */
                spin_unlock(&test_spinlock);
            }
            else
            {
                printk("unable to abtain spinlock ...\n");
            }

            printk("try spin lock done\n");
        break;
       default:
            printk("task: do nothing\n");
        break;
    }

    return 0;
}

static __exit void spinlock_demo_exit(void)
{
    printk("module removed!\n");
}

module_init(spinlock_demo_init);
module_exit(spinlock_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("kernel time operation");

