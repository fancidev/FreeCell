#ifndef FCPLAY_BTREE_H
#define FCPLAY_BTREE_H

typedef struct BTREENODE {
    int key;
    int left;
    int right;
} BTREENODE;

typedef struct BTREE {
    int min_key;        /* the smallest key */
    int max_key;        /* the biggest key */
    int root;           /* index of root node */
    BTREENODE *nodes;   /* memory of all nodes; node[0] is free_node */
    char *data;         /* user data, corresponding to nodes */
    int elemsize;       /* sizeof(user_data[0]) */
    int nalloc;         /* number of allocated pages */
    int count;          /* number of used nodes */
} BTREE;

#define NODES_PER_ALLOC 1000

#define BTree_GetMinKey(tree) ((tree)->min_key)
#define BTree_GetMaxKey(tree) ((tree)->max_key)
#define BTree_IsEmpty(tree)   ((tree)->root == 0)
#define BTree_Size(tree)      ((tree)->count)


BTREE * BTree_Create(int elemsize);
void BTree_Destroy(BTREE *tree);
//int  BTree_GetValue(BTREE *tree, int key);
//int  BTree_SetValue(BTREE *tree, int key, int value);
void* BTree_Insert(BTREE *tree, int key);
void* BTree_Lookup(BTREE *tree, int key);
void BTree_Delete(BTREE *tree, int key);
void BTree_PrintInfo(const BTREE *tree);
void BTree_GetKeys(const BTREE *tree, int *keys);

#endif