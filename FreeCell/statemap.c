#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "card.h"
#include "state.h"
#include "statemap.h"
#include "cardio.h"

#define GET_NODE(map,i) (&(map)->page[((i)-1)/STATES_PER_PAGE][((i)-1)%STATES_PER_PAGE])

typedef struct SCORE_ENTRY {
    int head;
    int last;
} SCORE_ENTRY;

static unsigned int compute_state_hash(const STATE *st)
{
    /*
    int i;
    unsigned __int64 h = 0;
    for (i = 0; i < 4; i++) {
        CARD c = TOP_CARD_OF_COLUMN(st,i);
        h = (h << 8) | ((unsigned int)c & 0xff);
    }
    return (unsigned int)(h % HASH_BUCKET_SIZE);
    */
    /*
    int i;
    unsigned int h1, h2;
    h1 = h2 = 0;
    for (i = 0; i < 4; i++) {
        CARD c1 = TOP_CARD_OF_COLUMN(st,i*2);
        CARD c2 = TOP_CARD_OF_COLUMN(st,i*2+1);
        h1 = (h1 << 8) | ((unsigned int)c1 & 0xff);
        h2 = (h2 << 8) | ((unsigned int)c2 & 0xff);
    }
    return (h1 ^ h2);
    */
    /*
    int i;
    unsigned int h1, h2;
    h1 = h2 = 0;
    for (i = 0; i < 4; i++) {
        CARD c1 = TOP_CARD_OF_COLUMN(st,i);
        CARD c2 = TOP_CARD_OF_COLUMN(st,i+4);
        h1 = (h1 << 6) | ((unsigned int)c1 & 0xff);
        h2 = (h2 << 6) | (((unsigned)c2<<4) & 0xf0) | (((unsigned)c2>>2) & 0x0f);
    }
    return (h1 ^ h2);
    */
    /*
    int i;
    unsigned int h1, h2, h3;
    h1 = h2 = h3 = 0;
    for (i = 0; i < 4; i++) {
        CARD c1 = TOP_CARD_OF_COLUMN(st,i);
        CARD c2 = TOP_CARD_OF_COLUMN(st,i+4);
        CARD c3 = CARD_IN_FREECELL(st,i);
        h1 = (h1 << 8) | ((unsigned int)c1 & 0xff);
        h2 = (h2 << 8) | (((unsigned)c2<<4) & 0xf0) | (((unsigned)c2>>4) & 0x0f);
        h3 = (h3 << 8) | (((unsigned)c3<<6) & 0xf0) | (((unsigned)c3>>2) & 0x0f);
    }
    return (h1 ^ h2 ^ h3);
    */
    /*
    unsigned int h1, h2, h3;
    h1 = (unsigned)(TOP_CARD_OF_COLUMN(st,0)) | 
         ((unsigned)TOP_CARD_OF_COLUMN(st,1)<<8) |
         ((unsigned)TOP_CARD_OF_COLUMN(st,2)<<16) |
         ((unsigned)TOP_CARD_OF_COLUMN(st,3)<<24);
    h2 = (unsigned)(TOP_CARD_OF_COLUMN(st,4)) | 
         ((unsigned)TOP_CARD_OF_COLUMN(st,5)<<8) |
         ((unsigned)TOP_CARD_OF_COLUMN(st,6)<<16) |
         ((unsigned)TOP_CARD_OF_COLUMN(st,7)<<24);
    h3 = ((unsigned)CARD_IN_FREECELL(st,0)) |
         ((unsigned)CARD_IN_FREECELL(st,1)<<8) |
         ((unsigned)CARD_IN_FREECELL(st,1)<<16) |
         ((unsigned)CARD_IN_FREECELL(st,1)<<24);
    h2 = ((h2<<4) & 0xf0f0f0f0) | ((h2>>4) & 0x0f0f0f0f);
    h3 = ((h3<<6) & 0xc0c0c0c0) | ((h3>>2) & 0x3f3f3f3f);
    return (h1 ^ h2 ^ h3);
    */
    unsigned int h1, h2, h3;
    h1 = *(unsigned*)(&st->head[0]);
    h2 = *(unsigned*)(&st->head[4]);
    h3 = *(unsigned*)(&st->hand[0]);
    h2 = ((h2<<4) & 0xf0f0f0f0) | ((h2>>4) & 0x0f0f0f0f);
    h3 = ((h3<<6) & 0xc0c0c0c0) | ((h3>>2) & 0x3f3f3f3f);
    return (h1 ^ h2 ^ h3);
    /*
    int i;
    unsigned int h1, h2;
    h1 = h2 = 0;
    for (i = 0; i < 4; i++) {
        CARD c1 = TOP_CARD_OF_COLUMN(st,i);
        CARD c2 = TOP_CARD_OF_COLUMN(st,i+4);
        h1 = (h1 << 8) | ((unsigned int)c1 & 0xff);
        h2 = (h2 << 8) | (((unsigned)c2<<4) & 0xf0) | (((unsigned)c2>>4) & 0x0f);
    }
    return (h1 ^ h2);
    */
}

STATEMAP* CreateStateMap(void)
{
    STATEMAP *map = (STATEMAP *)malloc(sizeof(STATEMAP));
    map->page = NULL;
    map->npage = 0;
    map->nlast = STATES_PER_PAGE;
    map->score_map = BTree_Create(sizeof(SCORE_ENTRY));
    map->nfresh = 0;
    memset(map->hash, 0, sizeof(map->hash));
    map->hash_hit = map->hash_miss = map->hash_hit_n = map->hash_miss_n = 0;
    return map;
}

void DestroyStateMap(STATEMAP *map)
{
    int i;
    for (i = 0; i < map->npage; i++) {
        free(map->page[i]);
    }
    BTree_Destroy(map->score_map);
    free(map->page);
    free(map);
}

int AddState(STATEMAP *map, const STATE *st, int parent, CARD_MOVE move, int score)
{
    STATENODE *node;
    /* int *ptr; */
    unsigned int hash;
    int bucket;
    int nodeIndex;
    SCORE_ENTRY *score_ent;

    /* alloc memory if page full */
    if (map->nlast >= STATES_PER_PAGE) {
        map->page = (STATENODE **)realloc(map->page, (++map->npage)*sizeof(STATENODE *));
        map->nlast = 0;
        map->page[map->npage-1] = (STATENODE*)malloc(sizeof(STATENODE)*STATES_PER_PAGE);
    }

    /* index of the new node */
    nodeIndex = (map->npage-1)*STATES_PER_PAGE + (++map->nlast);
    map->nfresh++;

    /* fill node */
    node = GET_NODE(map, nodeIndex);
    memmove(&node->st, st, sizeof(STATE));
    node->parent = parent;
    node->move = move;

    /* insert the node to the end of its score queue */
    score_ent = (SCORE_ENTRY*)BTree_Lookup(map->score_map, score);
    if (score_ent) {
        GET_NODE(map, score_ent->last)->next_score = nodeIndex;
        score_ent->last = nodeIndex;
    } else {
        score_ent = (SCORE_ENTRY*)BTree_Insert(map->score_map, score);
        score_ent->head = score_ent->last = nodeIndex;
    }
    node->next_score = 0;

    /* compute hash */
    hash = compute_state_hash(st);
    bucket = hash % HASH_BUCKET_SIZE;
    node->next_hash = map->hash[bucket];
    map->hash[bucket] = nodeIndex;

    /* return index of new node */
    return nodeIndex;
}

int GetParentState(const STATEMAP *map, int nodeIndex)
{
    return GET_NODE(map, nodeIndex)->parent;
}

int StateMapSize(const STATEMAP *map)
{
    if (map->npage == 0)
        return 0;
    else
        return (map->npage-1)*STATES_PER_PAGE+map->nlast;
}

const STATE * GetState(const STATEMAP *map, int i)
{
    return &GET_NODE(map,i)->st;
}

CARD_MOVE GetMove(const STATEMAP *map, int i)
{
    return GET_NODE(map,i)->move;
}

int NextState(STATEMAP *map)
{
    if (BTree_IsEmpty(map->score_map)) {
        return 0;
    } else {
        int score = BTree_GetMinKey(map->score_map);
        SCORE_ENTRY *score_ent = (SCORE_ENTRY*)BTree_Lookup(map->score_map, score);
        int index = score_ent->head;
        STATENODE *node = GET_NODE(map, index);
        if (node->next_score != 0) {
            score_ent->head = node->next_score;
        } else {
            BTree_Delete(map->score_map, score);
        }
        map->nfresh--;
        return index;
    }
}

int LookupStateMap(STATEMAP *map, const STATE *st)
{
    unsigned int hash = compute_state_hash(st);
    int bucket = hash % HASH_BUCKET_SIZE;
    int i, n = 0;

    for (i = map->hash[bucket]; i != 0; ) {
        STATENODE *node = GET_NODE(map, i);
        n++;
        if (memcmp(&node->st, st, sizeof(STATE)) == 0) {
            map->hash_hit_n += n;
            map->hash_hit++;
            return i;
        }
        i = node->next_hash;
    }
    map->hash_miss_n += n;
    map->hash_miss++;
    return 0;
}

int GetDepth(const STATEMAP *map, int index)
{
    int depth = 0;
    while (index) {
        depth++;
        index = GET_NODE(map,index)->parent;
    }
    return depth;
}

void PrintStateMap(const STATEMAP *map)
{
    int i, j, n;
    int hash_coverage, hash_max, hash_min;
    //int *scores, nscore, nexpand;

    printf("--------------------------------\n");
    printf("Statemap Info  \n");
    printf("--------------------------------\n");

    /* memory usage */
    printf("Memory usage...\n");
    printf("  Memory used:     %8dK\n", sizeof(STATENODE)*STATES_PER_PAGE*map->npage/1024);
    printf("  States per page: %8d\n", STATES_PER_PAGE);
    printf("  Allocated pages: %8d\n", map->npage);
    printf("  Allocated nodes: %8d\n", map->npage * STATES_PER_PAGE);
    printf("  Used nodes:      %8d\n", StateMapSize(map));
    printf("  Free nodes:      %8d\n", map->npage*STATES_PER_PAGE-StateMapSize(map));
    printf("\n");

    /* hash usage */
    hash_coverage = 0;
    hash_max = 0;
    hash_min = 99999999;
    for (i = 0; i < HASH_BUCKET_SIZE; i++) {
        j = map->hash[i];
        if (j > 0) {
            hash_coverage++;
            n = 0;
            while (j > 0) {
                ++n;
                j = GET_NODE(map,j)->next_hash;
            }
            if (n > hash_max) hash_max = n;
            if (n < hash_min) hash_min = n;
        }
    }
    printf("Hash usage...\n");
    printf("  Bucket size:     %8d\n", HASH_BUCKET_SIZE);
    printf("  # occupied:      %8d\n", hash_coverage);
    printf("  Average len:     %8.2f\n", (float)StateMapSize(map)/hash_coverage);
    printf("  Max len:         %8d\n", hash_max);
    printf("  Min len:         %8d\n", hash_min);
    printf("  # lookup:        %8d\n", map->hash_hit+map->hash_miss);
    printf("       hit:        %8d\n", map->hash_hit);
    printf("      miss:        %8d\n", map->hash_miss);
    printf("  Avg scan:        %8.2f\n", (float)(map->hash_hit_n+map->hash_miss_n)/
                                                (map->hash_hit+map->hash_miss));
    printf("       hit:        %8.2f\n", (float)map->hash_hit_n/map->hash_hit);
    printf("      miss:        %8.2f\n", (float)map->hash_miss_n/map->hash_miss);
    printf("\n");

    /* node expansion */
    /*
    nscore = BTree_Size(map->score_map);
    nexpand = 0;
    if (nscore > 0) {
        scores = (int*)malloc(sizeof(int)*nscore);
        BTree_GetKeys(map->score_map, scores);
        for (i = 0; i < nscore; i++) {
            j = BTree_GetValue(map->score_map, scores[i]);
            while (j) {
                nexpand++;
                j = GET_NODE(map,j)->next_score;
            }
        }
        free(scores);
    }
    */
    printf("Node expansion...\n");
    printf("  # expanded:       %8d\n", StateMapSize(map)-map->nfresh);
    //printf("  # fresh:          %8d\n", nexpand);
    printf("  # fresh (nfresh): %8d\n", map->nfresh);
    printf("\n");
    

    printf("Score map usage (BTree stat)...\n");
    BTree_PrintInfo(map->score_map);

    /*
    printf("Current head in score list...\n");
    if (BTree_IsEmpty(map->score_map)) {
        printf("None.\n");
    } else {
        int index = BTree_GetValue(map->score_map, BTree_GetMinKey(map->score_map));
        PrintState(&GET_NODE(map,index)->st);
    }
    */
}