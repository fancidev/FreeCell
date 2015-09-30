#ifndef FCPLAY_STATEMAP_H
#define FCPLAY_STATEMAP_H

#include "card.h"
#include "state.h"
#include "btree.h"

typedef int STATEMAPPOS;
#define STATEMAPPOS_NULL 0

typedef struct STATENODE {
    STATE       st;
    STATEMAPPOS parent;     /* position of parent node */
    CARD_MOVE   move;       /* the move from parent to this */
    STATEMAPPOS next_score; /* pos of next node with the same score */
    STATEMAPPOS next_hash;  /* pos of next node of the same hash */
} STATENODE;

/* Prime numbers are 10007,10009, 100003,200003 */
#define HASH_BUCKET_SIZE 200003

typedef struct STATEMAP {
    STATENODE **page;
    int npage;
    int nlast;          /* pointer to free node */
    BTREE *score_map;   /* ordered map, score => index of first node of this score */
    int nfresh;         /* number of fresh nodes, i.e. not NextState()-ed */
    STATEMAPPOS hash[HASH_BUCKET_SIZE]; /* pos of first node of the hash value */
    int hash_hit, hash_miss, hash_hit_n, hash_miss_n;
} STATEMAP;

#define STATES_PER_PAGE 4096


STATEMAP* CreateStateMap(void);
void DestroyStateMap(STATEMAP *map);
int AddState(STATEMAP *map, const STATE *st, int parent, CARD_MOVE move, int score);
int StateMapSize(const STATEMAP *map);
const STATE * GetState(const STATEMAP *map, int i);
int NextState(STATEMAP *map);
int LookupStateMap(STATEMAP *map, const STATE *st);
void PrintStateMap(const STATEMAP *map);
int GetParentState(const STATEMAP *map, int nodeIndex);
CARD_MOVE GetMove(const STATEMAP *map, int i);
int GetDepth(const STATEMAP *map, int index);


#endif
