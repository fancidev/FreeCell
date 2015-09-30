#include <stdio.h>
#include <stdlib.h>

#include "btree.h"


BTREE * BTree_Create(int elemsize)
{
    int i;
    BTREE *tree = (BTREE *)malloc(sizeof(BTREE));
    tree->min_key = 0;
    tree->max_key = 0;
    tree->root = 0;
    tree->nalloc = 1;
    tree->count = 0;
    tree->nodes = (BTREENODE *)malloc(sizeof(BTREENODE)*(1+NODES_PER_ALLOC));
    for (i = 0; i < NODES_PER_ALLOC; i++) {
        tree->nodes[i].right = i+1;
    }
    tree->nodes[i].right = 0;
    tree->elemsize = elemsize;
    tree->data = (char *)malloc(elemsize*(1+NODES_PER_ALLOC));
    return tree;
}

void BTree_Destroy(BTREE *tree)
{
    free(tree->nodes);
    free(tree->data);
    free(tree);
}

#define GET_DATA(tree,index) ((void *)((tree)->data+(tree)->elemsize*(index)))

void* BTree_Lookup(BTREE *tree, int key)
{
    int i;
    for (i = tree->root; i != 0; ) {
        BTREENODE *node = &tree->nodes[i];
        if (key < node->key) {
            i = node->left;
        } else if (key > node->key) {
            i = node->right;
        } else {
            return GET_DATA(tree, i);
        }
    }
    return NULL;
}

static int get_free_node(BTREE *tree)
{
    int i = tree->nodes[0].right;
    if (i == 0) {
        int n = NODES_PER_ALLOC * (++tree->nalloc);
        tree->nodes = (BTREENODE *)realloc(tree->nodes, (1+n)*sizeof(BTREENODE));
        for (i = tree->count + 1; i < n; i++) {
            tree->nodes[i].right = i+1;
        }
        tree->nodes[i].right = 0;
        tree->nodes[0].right = i = tree->count + 1;

        tree->data = (char *)realloc(tree->data, (1+n)*tree->elemsize);
    }
    tree->nodes[0].right = tree->nodes[i].right;
    tree->count++;
    return i;
}

void* BTree_Insert(BTREE *tree, int key)
{
    BTREENODE *node;
    int i;
    int *next = &tree->root;   /* pointer to the variable that points to the new node */
    int ismin = 1, ismax = 1;

    /* search for the proper place for this node */
    while (i = *next) {
        node = &tree->nodes[i];
        if (key < node->key) {
            next = &node->left;
            ismax = 0;
        } else if (key > node->key) {
            next = &node->right;
            ismin = 0;
        } else {
            return GET_DATA(tree,i);
        }
    }

    /* update statistic */
    if (ismin) tree->min_key = key;
    if (ismax) tree->max_key = key;

    /* add a new node */
    i = get_free_node(tree);
    node = &tree->nodes[i];
    node->key = key;
    node->left = node->right = 0;
    *next = i;

    return GET_DATA(tree,i);
}

static int find_max_in_subtree(const BTREE *tree, int root, int *parent)
{
    int last = 0, current, next;
    if (current = root) {
        while (next = tree->nodes[current].right) {
            last = current;
            current = next;
        }
    }
    if (parent) {
        *parent = last;
    }
    return current;
}

static int find_min_in_subtree(const BTREE *tree, int root, int *parent)
{
    int last = 0, current, next;
    if (current = root) {
        while (next = tree->nodes[current].left) {
            last = current;
            current = next;
        }
    }
    if (parent) {
        *parent = last;
    }
    return current;
}

void BTree_Delete(BTREE *tree, int key)
{
    BTREENODE *node;
    int i, newi;
    int *next = &tree->root;   /* pointer to the variable that points to the new node */
    int ismin = 1, ismax = 1;

    /* search for the node with the given key */
    while (i = *next) {
        node = &tree->nodes[i];
        if (key < node->key) {
            next = &node->left;
        } else if (key > node->key) {
            next = &node->right;
        } else {
            break;
        }
    }

    /* do nothing if this key doesn't exist */
    if (i == 0) {
        return;
    }

    /* now node points to the node to be removed */
    if (node->left == 0 && node->right == 0) {
        newi = 0;
    } else if (node->left == 0) {
        newi = node->right;
    } else if (node->right == 0) {
        newi = node->left;
    } else {
        int parent;
        newi = find_max_in_subtree(tree, node->left, &parent);
        tree->nodes[parent].right = tree->nodes[newi].left;
        tree->nodes[newi].left = node->left;
        tree->nodes[newi].right = node->right;
    }
    *next = newi;

    /* recycle the deleted node to free list */
    node->right = tree->nodes[0].right;
    tree->nodes[0].right = i;
    tree->count--;

    /* update minkey and maxkey */
    if (key == tree->min_key) {
        if (i = find_min_in_subtree(tree, tree->root, NULL)) 
            tree->min_key = tree->nodes[i].key;
        else
            tree->min_key = 0;
    }
    if (key == tree->max_key) {
        if (i = find_max_in_subtree(tree, tree->root, NULL))
            tree->max_key = tree->nodes[i].key;
        else
            tree->max_key = 0;
    }

    /* return */
    return;
}

/*
typedef int BTREEPOS;
#define BTREEPOS_NULL 0
#define BTree_GetRootPos(tree) ((tree)->root)
#define BTree_GetNode(tree,pos) (&(tree)->nodes[pos])
*/

/* Find the smallest value larger than key */
/*
int BTree_GetNextKey(const BTREE *tree, int key)
{
    BTREEPOS pos, parent = BTREEPOS_NULL;
    for (pos = BTree_GetRootPos(tree); pos != BTREEPOS_NULL; ) {
        BTREENODE *node = BTree_GetNode(tree, pos);
        if (key < node->key) {
            parent = pos;
            pos = node->left;
        } else if (key > node->key) {
            pos = node->right;
        } else {
            return node->value;
        }
    }
    return 0;
}
*/

int * traverse_keys(const BTREE *tree, int root, int *keys)
{
    BTREENODE *node = &tree->nodes[root];
    if (root) {
        if (node->left) {
            keys = traverse_keys(tree, node->left, keys);
        }
        *(keys++) = node->key;
        if (node->right) {
            keys = traverse_keys(tree, node->right, keys);
        }
    }
    return keys;
}

void BTree_GetKeys(const BTREE *tree, int *keys)
{
    traverse_keys(tree, tree->root, keys);
}


void BTree_PrintInfo(const BTREE *tree)
{
    printf("  Memory used:          %8dK\n", sizeof(BTREENODE)*tree->nalloc*NODES_PER_ALLOC/1024);
    printf("    sizeof(node):       %8d bytes\n", sizeof(BTREENODE));
    printf("    # nodes allocated:  %8d\n", tree->nalloc*NODES_PER_ALLOC);
    printf("    # nodes per page:   %8d\n", NODES_PER_ALLOC);
    printf("    # pages:            %8d\n", tree->nalloc);
    printf("  Nodes used:           %8d\n", tree->count);
}