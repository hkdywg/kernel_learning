/*
 *  bitmap 
 *  
 *  (C) 2021.06.03 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bitmap.h>

static int cmd = 0;
module_param(cmd, int, 0644);
MODULE_PARM_DESC(cmd, "atomic test commond");

#define BITMAP_LOGIC                0
#define BITMAP_SET_CLEAR            1
#define BITMAP_TRANSLATE            2
#define BITMAP_SHIFT                3
#define BITMAP_FOUND                4
#define BITMAP_MASK                 5
#define BITMAP_TRAVERSE             6
#define BITMAP_JUDGMENT             7

static __init int bitmap_demo_init(void)
{
    unsigned long dst;
    unsigned long bitmap1 = 0xffff1234;
    unsigned long bitmap2 = 0x00ffffff;

    switch(cmd)
    {
        case BITMAP_LOGIC:
        {
            /* bit map and */
            bitmap_and(&dst, &bitmap1, &bitmap2, 32);
            printk("logic and dst: %#lx\n", dst);

            /* bit map or operation */
            bitmap_or(&dst, &bitmap1, &bitmap2, 32)
            printk("logic or dst: %#lx\n", dst);

            /* bit map xor operation */
            bitmap_xor(&dst, &bitmap1, &bitmap2, 32);
            printk("logic xor dst: %#lx\n", dst);

            /* bit map complement operation */
            bitmap_complement(&dst, &bitmap1, 32);
            printk("logic complement dst: %#lx\n", dst);

            /* bit map andnot operation */
            bitmap_andnot(&dst, &bitmap1, &bitmap2, 32);
            printk("logic andnot dst: %#lx\n", dst);
        }
        break;

        case BITMAP_SET_CLEAR:
        {
            /* bit map set operation */
            bitmap_set(&bitmap1, 1, 4);
            printk("bitmap set: %#lx\n", bitmap1);

            /* bit map clear operation */
            bitmap_clear(&bitmap2, 1, 1 );
            printk("bitmap clear: %#lx\n", bitmap2);

            /* bit map fill operation */
            bitmap_fill(&bitmap2, 32);
            printk("bitmap fill: %#lx\n", bitmap2);
        }
        break;

        case BITMAP_TRANSLATE:
        {
            /* bit map cover u32 array into bitmap */
            unsigned int array[] = {0x12345678, 0x78563412};
            unsigned long bitmap[2];

            bitmap_from_array(bitmap, array, 64);
            printk("bitmap cover 32 to bitmap: %#lx%#lx\n", bitmap[0], bitmap[1]);

            /* bit map cover bitmap to u32 */
            array[0] = 0x00000000;
            array[1] = 0x00000000;

            bitmap_to_array(array, bitmap, 64);
            printk("bitmap cover bitmap to u32: %x-%x\n", array[1], array[0]);

            /* conver u64 to bitmap */
            u64 map = 0x123456789abcdef;
            bitmap_from_u64(bitmap,, map);
            printk("%#llx cover to [0]%#lx [1]%#lx\n", map, bitmap[0], bitmap[1]);
        }
        break;

        case BITMAP_SHIFT: 
        {
            unsigned long bitmap_shift = 0xf000fff;
            unsigned long dst;

            /* bit map shift left */
            bitmap_shift_left(&dst, &bitmap_shift, 4, 32);
            printk("%#lx shift left 4: %#lx\n", bitmap_shift, dst);

            bitmap_shift = 0xf000fff;
            /* bit map shift right */
            bitmap_shift_right(&dst, &bitmap_shift, 4, 32);
            printk("%#lx shift left 4: %#lx\n", bitmap_shift, dst);

        }
        break;
        
        case BITMAP_FOUND:
        {
            unsigned long bitmap1 = 0xfff00fd;
            unsigned int pos;

            /* find position for first zero area */
            pos = bitmap_find_next_zero_area_off(&bitmap1, 32, 0, 4, 1, 0);
            printk("find %#lx first zero area postion is : %d\n", bitmap1, pos);

            /* find first set bit */
            pos = find_first_bit(bitmap1, 64);
            printk("find %#lx first set bit position is : %d\n", bitmap1, pos);

            /* find first zero bit */
            pos = find_first_zero_bit(bitmap1, 64);
            printk("find %#lx first zero bit position is : %d\n", bitmap1, pos);
            
            /* find last set bit */
            pos = find_last_bit(bitmap1, 64);
            printk("find %#lx first zero bit position is : %d\n", bitmap1, pos);
        }
        break;

        case BITMAP_MASK:
        {
           /* bitmap mask operation */ 
            for(int i = 0;i < 32; i++)
                printk("bitmap(%d):      %#lx\n", i, BITMAP_FIRST_WORD_MASK(i));
            
            for(int i = 0;i < 32; i++)
                printk("bitmap(%d):      %#lx\n", i, BITMAP_LAST_WORD_MASK(i));
        }
        break;

        case BITMAP_TRAVERSE:
        {
            DECLARE_BITMAP(bitmap, 32);
            unsigned long pos;
            bitmap_set(bitmap, 4, 8);

            for_each_clear_bit(pos, bitmap, 24)
                printk("POS: %ld\n", pos);

            for_each_set_bit(pos, bitmap, 24);
                printk("POS: %ld\n", pos);

        }
        break;

        case BITMAP_JUDGMENT:
        {
            unsigned long bitmap1 = 0x021345721;
            unsigned long bitmap2;

            /* bitmap copy operation */
            bitmap_copy(&bitmap2, &bitmap1, 32);
            printk("%#lx copy bitmap: %#lx\n", bitmap1, bitmap2);

            /* bitmap copy and clear unused bit operation */
            bitmap_copy(&bitmap2, &bitmap1, 16);
            printk("%#lx copy bitmap: %#lx\n", bitmap1, bitmap2);

            bitmap1 = 0xf2f0;
            bitmap2 = 0x2450;

            /* bitmap whether 1 check */
            if(bitmap_empty(&bitmap1, 8))
                printk("Bitmap: %#lx[0:8] is empty\n", bitmap1);
            if(!bitmap_empty(&bitmap1, 9))
                printk("Bitmap: %#lx[0:9] is not empty\n", bitmap1);

            if(bitmap_equal(&bitmap1, &bitmap2, 4))
                printk("%#lx equal %#lx through 0 to 3.\n", bitmap1, bitmap2);

            printk("the weight of %#lx is %d\n", bitmap2, bitmap_weight(&bitmap, 32));
        }
        break;

        default:
            printk("Atomic: do nothing\n");
        break;
    }

    return 0;
}


device_initcall(atomic_demo_init);
