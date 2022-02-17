#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct list_node 
{
    int data;
    struct list_node *next;
};

struct tree_node
{
    int data;
    struct tree_node *left;
    struct tree_node *right;
};

static int Tree_data[] = {
                                        2, 1, 4, 6, -1, -1, 3, -1, -1, 6, 7,
                                        -1, -1, 9, -1, -1, 6, 6, 8, -1, -1, 2, -1,
                                        -1, 0, 1, -1, -1, 5, -1, -1
};

static int cnt = 0;
struct tree_node* create_tree(struct tree_node* node)
{
    int data = Tree_data[cnt++];
    if(data == -1)
        return NULL;
    node = (struct ntree_node*)malloc(sizeof(struct tree_node));
    node->data = data;

    node->left = create_tree(node->left);
    node->right = create_tree(node->right);
    return node;
}

int calculate_tree(struct tree_node* node)
{
    static int result = 0;
    if(node == NULL)
        return;
    result = result + node->data;
    calculate_tree(node->left);
    calculate_tree(node->right);
    return result;
}

void traverse_tree(struct tree_node* node)
{
    if(node == NULL)
        return;
    printf("node value = %d\n", node->data);
    traverse_tree(node->left);
    traverse_tree(node->right);
}

struct list_node* create_list(int value)
{
    struct list_node* list =  (struct list_node* )malloc(sizeof(struct list_node));
    list->data = value;
    list->next = NULL;

    return list;
}

int insert_list(struct list_node* list, struct list_node *node)
{
    struct list_node* tmp = list;
    if(list == NULL || node == NULL)
        return -1;
    while(tmp->next)
    {
        tmp = tmp->next;
    }
    tmp->next = node;
}

int insert_list_posion(struct list_node* list, int value, struct list_node* node)
{
    struct list_node* tmp = list;
    while(tmp != NULL)
    {
        if(tmp->next->data == value)
        {
            node->next = tmp->next->next;
            tmp->next = node;
            break;
        }
        tmp = tmp->next;
    }
}

int main(int argc, char *argv[])
{
    struct list_node *list = create_list(10);
    struct list_node list_node0 = {2, NULL};
    struct list_node list_node1 = {4, NULL};
    struct list_node list_node2 = {3, NULL};
    struct list_node list_node3 = {8, NULL};
    struct list_node list_node4 = {0, NULL};
    insert_list(list, &list_node0);
    insert_list(list, &list_node1);
    insert_list(list, &list_node2);
    insert_list(list, &list_node3);

    insert_list_posion(list, 3, &list_node4);

    for(;list;list=list->next)
    {
        printf("list node value %d\n", list->data);
    }

    struct tree_node* tree;
    tree = create_tree(tree);
    traverse_tree(tree);
    int result = calculate_tree(tree);
    printf("result = %d\n", result);
}
