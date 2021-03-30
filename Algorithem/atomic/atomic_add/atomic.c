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

static atomic_t viros_counter = ATOMIC_INIT(8);

static __init int atomic_demo_init(void)
{
    /* Atomic add */
    atomic_add(1, &viros_counter);

    printk("Atomic: %d\n", atomic_read(&viros_counter));

    return 0;
}


device_initcall(atomic_demo_init);
