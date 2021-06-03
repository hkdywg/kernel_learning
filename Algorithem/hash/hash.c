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
#include <linux/slab.h>
#include <linux/list.h>

/* number list head */
struct numlist
{
    struct hlist_head hlistHead;
};

/* number list node */
struct numnode
{
    int num;
    struct hlist_node hlistNode;
};

struct numlist nhead;
struct numnode nnode;

static __init int hash_demo_init(void)
{
    struct hlist_node *pos;
    struct numnode *listnode;
    int i;

    printk("hashlist is starting...\n");

    INIT_LIST_HEAD(&nhead.hlistHead);

    for(i = 0; i < MAX_NUM; i++)
    {
        listnode = (struct numnode *)kmalloc(sizeof(struct numnode), GFP_KERNEL);
        listnode->num = i + 1;

        hlist_add_head(&(listnode->hlistNode), &nhead.hlistHead);

        printk("Node %d had added to the hash list...\n", i + 1);
    }

    i = 1;

    struct numnode *p;

    hlist_for_each(pos, &nhead.hlistHead)
    {
        p = hlist_entry(pos, struct numnode, hlistNode);
        printk("Node %d data: %d\n", i, p->num);
        i++;
    }

    return 0;
}

static void __exit hash_demo_eit(void)
{
    struct hlist_node *pos, *n;
    struct numnode *p;
    int i;

    i = 1;

    hlist_for_each_safe(pos, n, &nhead.hlistHead)
    {
        hlist_del(pos);
        p = hlist_entry(pos, struct numnode, hlistNode);
        kfree(p);
        printk("Node %d has removed from the hashlist ...\n", i++);
    }
    printk("hash list is exiting ... \n");
}

module_init(hash_demo_init);
module_exit(hash_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg, hkdywg@163.com");
