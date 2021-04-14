/*
 *  klist
 *  
 *  (C) 2021.04.12 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */


/*
 *
 *  void klist_init(struct klist *k, void (*get)(struct klist_node *),
 *                                   void (*set)(struct klist_node *));
 *
 *  struct klist_node {
 *      void        *n_klist;
 *      struct list_head n_node;
 *      struct kref     n_ref;
 *  }
 *
 *  struct klist 
 *  {
 *      spinlock_t      k_lock;
 *      struct list_head    k_list;
 *      void            (*get)(struct klist_node *);
 *      void            (*set)(struct klist_node *);
 *  } __attribute__((aligned(sizeof(void *))));
 *
 *  Embedded bidirect-list between klist and klist_node.
 *
 *  +------------+             +--------------+            +-------------+
 *  |            |             |              |            |             |
 *  |  klist     |             |  klist_node  |            |  klist_node |
 *  |    k_list  |<----------->|       n_node |<---------->|      n_node |
 *  |            |             |              |            |             |
 *  |            |             |              |            |             |
 *  +------------+             +--------------+            +-------------+
 *
 * */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>

#include <linux/klist.h>

static int cmd = 0;
module_param(cmd, int, 0644);
MODULE_PARM_DESC(cmd, "spinlock test commond");

#define SPIN_LOCK_SAMPLE         (0x00)
#define SPIN_LOCK_IRQ            (0x01)
#define SPIN_LOCK_TRY            (0x02)

static struct klist *klist;

struct node {
    const char *name;
    struct klist_node n;
};

static struct node node0    =   {.name = "vir_node0",};
static struct node node1    =   {.name = "vir_node1",};
static struct node node2    =   {.name = "vir_node2",};
static struct node node3    =   {.name = "vir_node3",};
static struct node node4    =   {.name = "vir_node4",};
static struct node node5    =   {.name = "vir_node5",};

/* klist get */
static void demo_klist_get(struct klist_node *n)
{
    printk("Demo klist get. \n");
}

/* klist put */
static void demo_klist_put(struct klist_node *n)
{
    printk("Demo klist put. \n");
}

static __init int klist_demo_init(void)
{
    struct klist_node *np;
    struct node *nodep;
    struct klist_iter iter;

    switch(cmd)
    {
        case SPIN_LOCK_SAMPLE:
            klist = (struct klist *)kmalloc(sizeof(*klist), GFP_KERNEL);
            if(!klist)
                return -ENOMEM;
            /* Initialize klist */
            klist_init(klist, demo_klist_get, demo_klist_put);

            /* Initialize a klist_node and add it to back */
            klist_add_head(&node0.n, klist);

            /* Initialize a klist_node and add it to head */
            klist_add_tail(&node1.n, klist);
            klist_add_tail(&node2.n, klist);
            klist_add_tail(&node3.n, klist);
            klist_add_tail(&node4.n, klist);
            klist_add_tail(&node5.n, klist);

            /* Traverse all klist_node */
            printk("Traverse klist:\n");
            list_for_each_entry(np, &klist->k_list, n_node)
            {
                nodep = list_entry(np, struct node, n);
                if(nodep)
                    printk("%s\n", nodep->name);
            }

            /* Initialize a klist_iter structure */
            klist_iter_init_node(klist, &iter, &node0.n);

            /* Ante up next in list */
            while ((np = klist_next(&iter)))
            {
                nodep = list_entry(np, struct node, n);
                if (nodep)
                    printk("Next klist_node is: %s\n", nodep->name);
            }

            /* Finish a list iteration */
            klist_iter_exit(&iter);

            klist_remove(&node0.n);
            klist_remove(&node1.n);
            klist_remove(&node2.n);
            klist_remove(&node3.n);
            klist_remove(&node4.n);
            klist_remove(&node5.n);
            printk("list test done\n");
        break;
       default:
            printk("klist: do nothing\n");
        break;
    }

    return 0;
}

static __exit void klist_demo_exit(void)
{
    printk("module removed!\n");
}

module_init(klist_demo_init);
module_exit(klist_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("kernel time operation");

