/*
 *  init-call order
 *  
 *  (C) 2021.04.06 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */


#include <linux/kernel.h>
#include <linux/init.h>

#if 0
/* Early init */
static int  __early_initcall(void)
{
    printk("Early_initcall on first. \n");
    return 0;
}
early_initcall(__early_initcall);
#endif

/* Level 0 init */
int __init __pure_initcall(void)
{
    printk("Pure init call on level 0. \n");
    return 0;
}
pure_initcall(__pure_initcall);

/* Level 1 init */
int __init __core_initcall(void)
{
    printk("Core init call on level 1. \n");
    return 0;
}
core_initcall(__core_initcall);

/* Level 1s init */
int __init __core_initcall_sync(void)
{
    printk("Core init call sync on level 1s. \n");
    return 0;
}
core_initcall_sync(__core_initcall_sync);

/* Level 2 init */
int __init __postcore_initcall(void)
{
    printk("Post core init call on level 2. \n");
    return 0;
}
postcore_initcall(__postcore_initcall);

/* Level 2s init */
int __init __postcore_initcall_sync(void)
{
    printk("Post core init call sync on level 2s. \n");
    return 0;
}
postcore_initcall_sync(__postcore_initcall_sync);

/* Level 3 init */
int __init __arch_initcall(void)
{
    printk("Arch init call on level 3. \n");
    return 0;
}
arch_initcall(__arch_initcall);

/* Level 3s init */
int __init __arch_initcall_sync(void)
{
    printk("Arch init call sync on level 3s. \n");
    return 0;
}
arch_initcall_sync(__arch_initcall_sync);

/* Level 4 init */
int __init __subsys_initcall(void)
{
    printk("Subsys init call on level 4. \n");
    return 0;
}
subsys_initcall(__subsys_initcall);


/* Level 4s init */
int __init __subsys_initcall_sync(void)
{
    printk("Subsys init call sync on level 4s. \n");
    return 0;
}
subsys_initcall_sync(__subsys_initcall_sync);


/* Level 5 init */
int __init __fs_initcall(void)
{
    printk("Fs init call on level 5. \n");
    return 0;
}
fs_initcall(__fs_initcall);


/* Level 5x init */
int __init __rootfs_initcall(void)
{
    printk("Rootfs init call on level 5x. \n");
    return 0;
}
rootfs_initcall(__rootfs_initcall);


/* Level 6 init */
int __init __device_initcall(void)
{
    printk("Device init call on level 6. \n");
    return 0;
}
device_initcall(__device_initcall);



/* Level 6s init */
int __init __device_initcall_sync(void)
{
    printk("Device init call sync on level 6s. \n");
    return 0;
}
device_initcall_sync(__device_initcall_sync);


/* Level 7 init */
int __init __late_initcall(void)
{
    printk("Late init call on level 7. \n");
    return 0;
}
late_initcall(__late_initcall);


/* Level 7s init */
int __init __late_initcall_sync(void)
{
    printk("Late init call sync on level 7s. \n");
    return 0;
}
late_initcall_sync(__late_initcall_sync);


