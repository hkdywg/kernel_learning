/*
 *  atomic
 *  
 *  (C) 2021.03.29 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */

/* Memory access
 *
 *         +------+
 *         |      |
 *         |      |
 *         | CPU  |
 *         |      |
 *         |      |
 *         +------+
 *            A
 *            |
 *            |                                                         +--------+
 *            |                                                         |        |
 *            |                                                         |        |
 *            |                                                         |        |
 *  +------+  |             +------------+          +------------+      |        |
 *  |      |  |             |            |          |            |      |        |
 *  |      |                |            |          |            |      |        |
 *  | CPU  |<-------------->|  L1 Cache  |<-------->|  L2 Cache  |<---->| Memory |
 *  |      |       A        |            |          |            |      |        |
 *  |      |       |        |            |          |            |      |        |
 *  +------+       |        +------------+          +------------+      |        |
 *                 |                                                    |        |
 *                 |                                                    |        |
 *                 |--------------------------------------------------->|        |                                                           
 *                                volatile/atomic                       |        |
 *                                                                      +--------+
 *
 **/


#include <linux/kernel.h>
#include <linux/module.h>

static int cmd = 0;
module_param(cmd, int, 0644);
MODULE_PARM_DESC(cmd, "atomic test commond");

static atomic_t viros_counter = ATOMIC_INIT(8);

#define ATOMIC_ADD                  0
#define ATOMIC_AND                  1
#define ATOMIC_SET                  2
#define ATOMIC_READ                 3
#define ATOMIC_SUB                  4
#define ATOMIC_INC                  5
#define ATOMIC_DEC                  6
#define ATOMIC_ANDNOT               7

static __init int atomic_demo_init(void)
{
    switch(cmd)
    {
        case ATOMIC_ADD:
        /* Atomic add */
        atomic_add(10, &viros_counter);

        printk("Atomic add: %d\n", atomic_read(&viros_counter));
        break;
        case ATOMIC_AND:
        atomic_set(&viros_counter, 10);
        /* Atomic and*/
        atomic_and(7, &viros_counter);

        printk("Atomic and: %d\n", atomic_read(&viros_counter));
        break;
        case ATOMIC_SET:
        /* Atomic set*/
        atomic_set(&viros_counter, 5);

        printk("Atomic set: %d\n", atomic_read(&viros_counter));
        break;
        case ATOMIC_READ:
        /* Atomic read*/
        printk("Atomic read: %d\n", atomic_read(&viros_counter));
        break;
        case ATOMIC_SUB:
        /* Atomic sub*/
        atomic_sub(10, &viros_counter);

        printk("Atomic sub: %d\n", atomic_read(&viros_counter));
        break;
        case ATOMIC_INC:
        /* Atomic inc*/
        atomic_inc(&viros_counter);

        printk("Atomic inc: %d\n", atomic_read(&viros_counter));
        break;
        case ATOMIC_DEC:
        /* Atomic */
        atomic_dec(&viros_counter);

        printk("Atomic dec: %d\n", atomic_read(&viros_counter));
        break;
        case ATOMIC_ANDNOT:
        /* Atomic */
        atomic_andnot(5, &viros_counter);

        printk("Atomic andnot: %d\n", atomic_read(&viros_counter));
        break;
        default:
            printk("Atomic: do nothing\n");
        break;
    }

    return 0;
}


device_initcall(atomic_demo_init);
