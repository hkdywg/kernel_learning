/*
 *  binary  
 *  
 *  (C) 2021.05.17 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */

#include <stdio.h>
#include <stdlib.h>

/* binary-tree node */
struct binary_node {
    int idx;
    struct binary_node *left;
    struct binary_node *right;
};


static int Perfect_BinaryTree_data[] = {
                                        200, 143, 754, 6, -1, -1, 3, -1, -1, 386, 7,
                                        -1, -1, 9, -1, -1, 876, 486, 8, -1, -1, 2, -1,
                                        -1, 740, 1, -1, -1, 5, -1, -1
};

static int counter = 0;
static int *BinaryTree_data = Perfect_BinaryTree_data;

/* Preoder Create Binary-tree */
static struct binary_node *Preorder_Create_BinaryTree(struct binary_node *node)
{
    int ch = BinaryTree_data[counter++];

    /* input from terminal */
    if(ch == -1)
    {
        return NULL;
    }
    else
    {
        node = (struct binary_node *)malloc(sizeof(struct binary_node));
        node->idx = ch;

        /* create left child */
        node->left = Preorder_Create_BinaryTree(node->left);
        /* create right child */
        node->right = Preorder_Create_BinaryTree(node->right);
        return node;
    }
}

/* Postorder Create Binary-tree */
static struct binary_node *Postorder_Create_BinaryTree(struct binary_node *node)
{
    int ch = BinaryTree_data[counter++];

    /* input from terminal */
    if(ch == -1)
    {
        return NULL;
    }
    else
    {
        node = (struct binary_node *)malloc(sizeof(struct binary_node));
        node->idx = ch;

        /* create right child */
        node->right = Postorder_Create_BinaryTree(node->right);
        /* create left child */
        node->left = Postorder_Create_BinaryTree(node->left);
        return node;
    }
}



/* Pre-Traverse Binary-Tree */
static void Preorder_Traverse_BinaryTree(struct binary_node *node)
{
    if(node == NULL)
    {
        return;
    }
    else
    {
        printf("%d ", node->idx);
        /* Traverse left node */
        Preorder_Traverse_BinaryTree(node->left);
        /* Traverse right node */
        Preorder_Traverse_BinaryTree(node->right);
    }
}

/* Midd-Traverse Binary-Tree */
static void Middorder_Traverse_BinaryTree(struct binary_node *node)
{
    if(node == NULL)
    {
        return;
    }
    else
    {
        /* Traverse left node */
        Middorder_Traverse_BinaryTree(node->left);
        printf("%d ", node->idx);
        /* Traverse right node */
        Middorder_Traverse_BinaryTree(node->right);
    }
}

/* Post-Traverse Binary-Tree */
static void Postorder_Traverse_BinaryTree(struct binary_node *node)
{
    if(node == NULL)
    {
        return;
    }
    else
    {
        /* Traverse left node */
        Postorder_Traverse_BinaryTree(node->left);
        /* Traverse right node */
        Postorder_Traverse_BinaryTree(node->right);
        printf("%d ", node->idx);
    }
}

/* The deep for Binary-node */
static int BinaryTree_Deep(struct binary_node *node)
{
    int deep = 0;
    if(node != NULL)
    {
        int leftdeep = BinaryTree_Deep(node->left);
        int rightdeep = BinaryTree_Deep(node->right);

        deep = leftdeep >= rightdeep ? leftdeep + 1 : rightdeep + 1;
    }
    return deep;
}

/* leaf counter */
static int BinaryTree_LeafCount(struct binary_node *node)
{
    static int count;

    if(node != NULL)
    {
        if(node->left == NULL && node->right == NULL)
            count++;

        BinaryTree_LeafCount(node->left);
        BinaryTree_LeafCount(node->right);
    }
    return count;
}

/* Post-Free BinaryTree */
static void Postorder_Free_BinaryTree(struct binary_node *node)
{
    if(node == NULL)
        return;
    else
    {
        Postorder_Free_BinaryTree(node->left);
        Postorder_Free_BinaryTree(node->right);
        free(node);
        node = NULL;
    }
}

int main()
{
    /* Define binary-tree root */
    struct binary_node *basic_root;

    printf("Preorder Create BinaryTree\n");
    basic_root =  Preorder_Create_BinaryTree(basic_root);

    /* Preoder traverse binary-tree */
    printf("Preorder traverse binary-tree\n");
    Preorder_Traverse_BinaryTree(basic_root);
    printf("\n");

    /* Middorder traverse binary-tree */
    printf("Middorder traverse binary-tree\n");
    Middorder_Traverse_BinaryTree(basic_root);
    printf("\n");

    /* Postorder traverse binary-tree */
    printf("Postorder traverse binary-tree\n");
    Postorder_Traverse_BinaryTree(basic_root);
    printf("\n");

    /* The deep of binary-tree */
    printf("The binary-tree deep: %d\n", BinaryTree_Deep(basic_root));

    /* The leaf number for binary-tree */
    printf("The binary-tree leaf: %d\n", BinaryTree_LeafCount(basic_root));

    /* Postorder free binary-tree */
    Postorder_Free_BinaryTree(basic_root);

    return 0;
}
