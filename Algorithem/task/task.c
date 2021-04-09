/*
 *  task
 *  
 *  (C) 2021.03.29 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>

static int cmd = 0;
module_param(cmd, int, 0644);
MODULE_PARM_DESC(cmd, "task test commond");

#define QUERY_THREAD_INFO       (0)

static __init int thread_demo_init(void)
{
    struct thread_info *info;
    unsigned long stack;

    switch(cmd)
    {
        case QUERY_THREAD_INFO:
            /* obtain current thread_info address */
            info = current_thread_info();

            /* obtain stack address */
            stack = (__force unsigned long)info;
            stack += THREAD_SIZE;

            printk("thread_info Address: %#lx\n", (__force unsigned long)info);
            printk("stack address: %#lx\n", current_stack_pointer);
            printk("thread_union end address: %#lx\n", stack);
        break;
       default:
            printk("task: do nothing\n");
        break;
    }

    return 0;
}

static __exit void thread_demo_exit(void)
{
    printk("module removed!\n");
}

module_init(thread_demo_init);
module_exit(thread_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("kernel time operation");

