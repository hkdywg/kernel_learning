/*
 * Single list demo code 0.
 *
 * (C) 2021.05.25 <hkdywg@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <stdio.h>
#include <stdlib.h>

struct node {
    struct node *next;
    int num;
};

static void single_list_insert(struct node ***list, struct node *node)
{
    **list = node;
    *list = &node->next;
}

int main()
{
    static struct node n0, n1, n2, n3;
    struct node **node_list = NULL;
    struct node *tmp;

    n0.num = 0x00;
    n1.num = 0x01;
    n2.num = 0x02;
    n3.num = 0x03;

    /* node list init */
    node_list = &n0.next;
    single_list_insert(&node_list, &n0);
    single_list_insert(&node_list, &n1);
    single_list_insert(&node_list, &n2);
    single_list_insert(&node_list, &n3);
    
    for(tmp = &n0; tmp; tmp = tmp->next)
        printf("%d\n", tmp->num);

#if 1
    /* node list init */
    node_list = &n0.next;

    /* Insert 1st node */
    *node_list = &n0;
    node_list = &n0.next;

    /* Insert 2st node */
    *node_list = &n1;
    node_list = &n1.next;

    /* Insert 3st node */
    *node_list = &n2;
    node_list = &n2.next;

    /* Insert 4th node */
    *node_list = &n3;
    node_list = &n3.next;
#else
    n0.next = &n1;
    n1.next = &n2;
    n2.next = &n3;
#endif

    for(tmp = &n0; tmp; tmp = tmp->next)
        printf("%d\n", tmp->num);

    return 0;
}
