/*
 *  kobject
 *  
 *  (C) 2021.04.07 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kobject.h>

#include <linux/rbtree.h>
#include <linux/rbtree_augmented.h>

#include <linux/slab.h>

static int cmd = 0;
module_param(cmd, int, 0644);
MODULE_PARM_DESC(cmd, "kobject test commond");

#define KOBJECT_CREATE_AND_ADD      (0x00)
#define KOBJECT_GET                 (0x01)
#define KOBJECT_GET_PATH            (0x02)
#define KOBJECT_INIT                (0x04)
#define KOBJECT_PUT                 (0x08)


struct demo_dev{
    struct kobject kobj;
    int dev_size;
};

struct demo_sysfs_entry{
    struct attribute attr;
    ssize_t (*show)(struct demo_dev *, char *);
    ssize_t (*store)(struct demo_dev *, const char *, size_t);
};

static struct demo_dev *bdev;

static struct kobject *kobj;
extern struct kernfs_node *sysfs_root_kn;

#define rb_to_kn(x) rb_entry((x), struct kernfs_node, rb)

static void demo_free(struct kobject *kobj)
{
    struct demo_dev *bdev = container_of(kobj, struct demo_dev, kobj);
    
    kfree(bdev);

    bdev = NULL;
}

static ssize_t demo_attr_show(struct kobject *kobj, struct attribute *attr, char *page)
{
    return 0;
}

static ssize_t demo_attr_store(struct kobject *kobt, struct attribute *attr, const char *page, size_t length)
{
    return 0;
}

static ssize_t size_show(struct demo_dev *bdev, char *page)
{
    return sprintf(page, "%d\n", (int)bdev->dev_size);
}

static ssize_t size_store(struct demo_dev *bdev, const char *buf, size_t len)
{
    sscanf(buf, "%d", &bdev->dev_size);
    return len;
}

static const struct sysfs_ops demo_sysfs_ops = {
    .show   = demo_attr_show,
    .store  = demo_attr_store,
};

static struct demo_sysfs_entry demo_size = __ATTR(VirOs_size, S_IRUGO | S_IWUSR, size_show, size_store);

static struct attribute *demo_default_attrs [] = {
    &demo_size.attr,
    NULL,
};

/* kset */
static struct kobj_type demo_ktype = {
    .release = demo_free,
    .sysfs_ops = &demo_sysfs_ops,
    .default_attrs = demo_default_attrs,
};

static __init int kobject_demo_init(void)
{
    int ret;
    struct kernfs_node *kn, *pos;
    struct rb_root *rbroot;
    struct rb_node *np;
    char *path = NULL;

    switch(cmd)
    {
        case KOBJECT_CREATE_AND_ADD:
            kobj = kobject_create_and_add("VirOs", NULL);
            if(!kobj)
                return -ENOMEM;
            
            kn = kobj->sd;
            printk("Kobject: %s\n", kobject_name(kobj));

            /* parent rbtree */
            rbroot = &kn->parent->dir.children;

            /* traverser all knode on rbtree )*/
            for(np = rb_first_postorder(rbroot); np;
                np = rb_next_postorder(np))
            {
                pos = rb_to_kn(np);
                printk("Name: %s\n", pos->name);
            }
        break;

        case KOBJECT_GET:
            kobj = kobject_create_and_add("VirOs", NULL);
            if(!kobj)
                return -ENOMEM;

            printk("Kobject kref0: %d\n", kref_read(&kobj->kref));
            kobj = kobject_get(kobj);
            if(!kobj)
            {
                printk("Error: kobject_get ... \n");
                ret = -EINVAL;
                goto err_get;
            }
            printk("Kobject kref1: %d\n", kref_read(&kobj->kref));
        break;

        case KOBJECT_GET_PATH:
            kobj = kobject_create_and_add("VirOs", NULL);
            if(!kobj)
                return -ENOMEM;

            kobj = kobject_get(kobj);
            if(!kobj)
            {
                printk("Error: kobject_get ... \n");
                ret = -EINVAL;
                goto err_get;
            }

            path = kobject_get_path(kobj, GFP_KERNEL);
            if(!path)
            {
                printk("Error: kobject_get_path \n");
                ret = -ENOMEM;
                goto err_get;
            }
            printk("VirOs path: /sys%s\n", path);
        break;

        case KOBJECT_INIT:
        {
            printk("init kobject ...\n");
            bdev = kzalloc(sizeof(*bdev), GFP_KERNEL);
            if(!bdev)
            {
                printk("Error: kzalloc()\n");
                ret = -ENOMEM;
                goto err_all;
            }
            kobject_init(&bdev->kobj, &demo_ktype);
        }
        break;
        default:
            printk("timing: do nothing\n");
        break;
    }

err_get:
    kobject_put(kobj);
    return 0;

err_all:
    return ret;

    return ret;
}

static __exit void kobject_demo_exit(void)
{
    if(bdev)
    {
        kfree(bdev);
        return;
    }
    kobject_put(kobj);
    printk("module removed!\n");
}

module_init(kobject_demo_init);
module_exit(kobject_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("kernel time operation");

