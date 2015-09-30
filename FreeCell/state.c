#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "card.h"
#include "state.h"

void GetBottomCards(const STATE *st, CARD cards[8])
{
    int i;
    for (i = 0; i < 8; i++) {
        CARD c, c2 = CARD_NONE;
        for (c = TOP_CARD_OF_COLUMN(st, i); IS_CARD(c); c = CARD_UNDER(st, c)) {
            c2 = c;
        }
        cards[i] = c2;
    }
}

static void sort_cards(CARD cards[], int n, CARD sort2[])
{
    int i, j;
    for (i = 0; i < n-1; i++) {
        for (j = i+1; j < n; j++) {
            if (cards[i] > cards[j]) {
                CARD ct = cards[i];
                cards[i] = cards[j];
                cards[j] = ct;
                if (sort2) {
                    ct = sort2[i];
                    sort2[i] = sort2[j];
                    sort2[j] = ct;
                }
            }
        }
    }
}

static void reorder_columns(STATE *st)
{
    CARD root[8];
    GetBottomCards(st, root);
    sort_cards(root, 8, st->head);
}

void ResetState(STATE *st)
{
    memset(st, 0, sizeof(STATE));
    CARD_IN_HOMECELL(st,CARD_SPADE) = MAKE_CARD(CARD_NONE,CARD_SPADE);
    CARD_IN_HOMECELL(st,CARD_HEART) = MAKE_CARD(CARD_NONE,CARD_HEART);
    CARD_IN_HOMECELL(st,CARD_CLUB) = MAKE_CARD(CARD_NONE,CARD_CLUB);
    CARD_IN_HOMECELL(st,CARD_DIAMOND) = MAKE_CARD(CARD_NONE,CARD_DIAMOND);
}

void NormalizeState(STATE *st)
{
    reorder_columns(st);
}

void MoveCard(STATE *st, CARD_MOVE m)
{
    CARD c = MOVE_WHAT(m);
    CARD_LOCATION from = MOVE_SRC(m);
    CARD_LOCATION to = MOVE_DST(m);

    int i, j;
    CARD topcard = CARD_NONE;
    int removed_col = -1;

    switch (LOCATION_TYPE(from)) {
    case LOC_FREECELL:
        i = LOCATION_INDEX(from);

        /* empty this free cell */
        for (j = i; j > 0; j--) {
            CARD_IN_FREECELL(st,j) = CARD_IN_FREECELL(st,j-1);
        }
        CARD_IN_FREECELL(st,0) = CARD_NONE;

        break;

    case LOC_COLUMN:
        i = LOCATION_INDEX(from);

        /* remove cards from this column */
        topcard = TOP_CARD_OF_COLUMN(st,i);
        TOP_CARD_OF_COLUMN(st,i) = CARD_UNDER(st,c);

        /* sort the columns if necessary */
        if (IS_COLUMN_EMPTY(st,i)) {
            for (j = i; j > 0; j--) {
                TOP_CARD_OF_COLUMN(st,j) = TOP_CARD_OF_COLUMN(st,j-1);
            }
            TOP_CARD_OF_COLUMN(st,0) = CARD_NONE;
            removed_col = i;
        }
        break;
    }

    switch (LOCATION_TYPE(to)) {
    case LOC_HOMECELL:
        CARD_UNDER(st,c) = CARD_IN_HOMECELL(st,c);
        CARD_IN_HOMECELL(st,c) = c;
        break;

    case LOC_FREECELL:
        CARD_UNDER(st,c) = CARD_NONE;

        /* find the proper cell to insert the card */
        for (i = 3; (i >= 0) && CARD_IN_FREECELL(st,i) > c; i--);
        if (i < 0) {
            /* this is illegal, but we put up with it */
            i = 0;
        }

        /* insert the card to freecell[i], moving others forward */
        for (j = 0; j < i; j++) {
            CARD_IN_FREECELL(st,j) = CARD_IN_FREECELL(st,j+1);
        }
        CARD_IN_FREECELL(st,i) = c;
        break;

    case LOC_COLUMN:
        i = LOCATION_INDEX(to);
        if (i < removed_col) {
            i++;
        }

        /* if that column was empty, we need to reorder */
        if (IS_COLUMN_EMPTY(st,i)) {
            CARD root[8];
            GetBottomCards(st, root);
            for (i = 7; (i >= 0) && (root[i] > c); i--);
            if (i < 0) { /* illegal move, put up with it anyway */
                i = 0;
            }
            for (j = 0; j < i; j++) {
                TOP_CARD_OF_COLUMN(st,j) = TOP_CARD_OF_COLUMN(st,j+1);
            }
            TOP_CARD_OF_COLUMN(st,i) = CARD_NONE;
        }

        /* put this card to the top of the column */
        CARD_UNDER(st,c) = TOP_CARD_OF_COLUMN(st,i);
        TOP_CARD_OF_COLUMN(st,i) = IS_CARD(topcard)? topcard:c;
        break;
    }
}

#if 0
/* Move _card_ from location _from_ to location _to_, assuming
 * such move is legal.
 */
void MoveCard2(STATE *st, CARD c, CARD_LOCATION from, CARD_LOCATION to)
{
    int i, j;
    CARD topcard = CARD_NONE;
    int removed_col = -1;

    switch (LOCATION_TYPE(from)) {
    case LOC_FREECELL:
        i = LOCATION_INDEX(from);

        /* empty this free cell */
        for (j = i; j > 0; j--) {
            CARD_IN_FREECELL(st,j) = CARD_IN_FREECELL(st,j-1);
        }
        CARD_IN_FREECELL(st,0) = CARD_NONE;

        break;

    case LOC_COLUMN:
        i = LOCATION_INDEX(from);

        /* remove cards from this column */
        topcard = TOP_CARD_OF_COLUMN(st,i);
        TOP_CARD_OF_COLUMN(st,i) = CARD_UNDER(st,c);

        /* sort the columns if necessary */
        if (IS_COLUMN_EMPTY(st,i)) {
            for (j = i; j > 0; j--) {
                TOP_CARD_OF_COLUMN(st,j) = TOP_CARD_OF_COLUMN(st,j-1);
            }
            TOP_CARD_OF_COLUMN(st,0) = CARD_NONE;
            removed_col = i;
        }
        break;
    }

    switch (LOCATION_TYPE(to)) {
    case LOC_HOMECELL:
        CARD_UNDER(st,c) = CARD_IN_HOMECELL(st,c);
        CARD_IN_HOMECELL(st,c) = c;
        break;

    case LOC_FREECELL:
        CARD_UNDER(st,c) = CARD_NONE;

        /* find the proper cell to insert the card */
        for (i = 3; (i >= 0) && CARD_IN_FREECELL(st,i) > c; i--);
        if (i < 0) {
            /* this is illegal, but we put up with it */
            i = 0;
        }

        /* insert the card to freecell[i], moving others forward */
        for (j = 0; j < i; j++) {
            CARD_IN_FREECELL(st,j) = CARD_IN_FREECELL(st,j+1);
        }
        CARD_IN_FREECELL(st,i) = c;
        break;

    case LOC_COLUMN:
        i = LOCATION_INDEX(to);
        if (i < removed_col) {
            i++;
        }

        /* if that column was empty, we need to reorder */
        if (IS_COLUMN_EMPTY(st,i)) {
            CARD root[8];
            GetBottomCards(st, root);
            for (i = 7; (i >= 0) && (root[i] > c); i--);
            if (i < 0) { /* illegal move, put up with it anyway */
                i = 0;
            }
            for (j = 0; j < i; j++) {
                TOP_CARD_OF_COLUMN(st,j) = TOP_CARD_OF_COLUMN(st,j+1);
            }
            TOP_CARD_OF_COLUMN(st,i) = CARD_NONE;
        }

        /* put this card to the top of the column */
        CARD_UNDER(st,c) = TOP_CARD_OF_COLUMN(st,i);
        TOP_CARD_OF_COLUMN(st,i) = IS_CARD(topcard)? topcard:c;
        break;
    }

}
#endif

/* Check if we can safely collect card _c_.
 * Returns nonzero if we can, or zero if we cannot.
 */
int CanCollectSafely(const STATE *st, CARD c)
{
    int collect_ok = 0;
    if (RANK_OF(c) == CARD_A) {
        collect_ok = 1;
    } else {
        CARD c0 = CARD_IN_HOMECELL(st,c);
        if (IS_UP_INSUIT(c,c0)) {
            if (RANK_OF(c) <= 2) {
                collect_ok = 1;
            } else {
                /* If both suits of the alternate color have
                 * collected cards ranking at least (N-2), 
                 * then we can safely collect this card.
                 */
                if ((RANK_OF(CARD_IN_HOMECELL(st,ALT_SUIT_1(c))) >= RANK_OF(c) - 2) &&
                    (RANK_OF(CARD_IN_HOMECELL(st,ALT_SUIT_2(c))) >= RANK_OF(c) - 2)) {
                    collect_ok = 1;
                }
            }
        }
    }
    return collect_ok;
}

/* Collect as many cards to home cells as possible, as long as
 * such is safe. A.k.a. maximal safe autoplay.
 *
 * Automatically normalize the state after collection.
 *
 * Returns the number of cards collected.
 */
#if 0
int CollectSafely(STATE *st)
{
    int i, n, tot = 0;
    do {
        n = 0;
        /* try collect cards in the tableau */
        for (i = 0; i < 8; i++) {
            CARD c = TOP_CARD_OF_COLUMN(st,i);
            if (IS_CARD(c) && CanCollectSafely(st, c)) {
                MoveCard2(st, c, LOC_COLUMN+i, LOC_HOMECELL);
                ++n;
            }
        }
        /* try collect cards in the free cells */
        for (i = 0; i < 4; i++) {
            CARD c = CARD_IN_FREECELL(st, i);
            if (IS_CARD(c) && CanCollectSafely(st, c)) {
                MoveCard2(st, c, LOC_FREECELL+i, LOC_HOMECELL);
                ++n;
            }
        }
        tot += n;
    } while (n > 0);
    return tot;
}
#endif

int CollectSafely(STATE *st)
{
    int i, n, tot = 0;

    do {
        n = 0;
        /* try collect cards in the tableau */
        for (i = 0; i < 8; i++) {
            CARD c = TOP_CARD_OF_COLUMN(st,i);
            if (CAN_COLLECT(st,c) && CanCollectSafely(st, c)) {
                MoveCard(st, PACK_MOVE(c,LOC_COLUMN+i,LOC_HOMECELL));
                ++n;
            }
        }
        /* try collect cards in the free cells */
        for (i = 0; i < 4; i++) {
            CARD c = CARD_IN_FREECELL(st, i);
            if (CAN_COLLECT(st,c) && CanCollectSafely(st, c)) {
                MoveCard(st, PACK_MOVE(c,LOC_FREECELL+i,LOC_HOMECELL));
                ++n;
            }
        }
        tot += n;
    } while (n > 0);
    return tot;
}

/* Find out all possible moves from the given state, including supermoves,
 * and store them in max_moves. Store no more than max_moves items.
 * Return the number of possible moves, which may > max_moves if max_moves
 * is not sufficient.
 *
 * CAUTION: state must be normalized before calling this function.
 *
 * Note: There should be 80 room for moves, calculated as follows:
 *  - At most 8 cards could be moved into free cell.
 *  - At most 4 cards could be moved into home cell.
 *  - At most 52 cards could be moved into an empty column.
 *  - At most 16 cards could be moved onto a front card.
 * These add up to 8+4+52+16=80.
 */
int FindPossibleMoves(const STATE *st, CARD_MOVE moves[], int max_moves)
{
    int i, n;
    int max_move_nonroot, max_move_root;
    int nEmptyCell, nEmptyColumn;
    unsigned char /*CARD_LOCATION*/ front_card_loc[52];

    /* how many empty free cells are there? */
    for (nEmptyCell = 0; (nEmptyCell < 4) && IS_FREECELL_EMPTY(st,nEmptyCell); nEmptyCell++);

    /* how many empty columns are there? */
    for (nEmptyColumn = 0; (nEmptyColumn < 8) && IS_COLUMN_EMPTY(st,nEmptyColumn); nEmptyColumn++);
    
    /* so, how many cards can we move at a time? */
#define calc_move(n1,n2) ((n1)*((n2)+1)+1+(n2)*((n2)+1)/2)
    //max_move_nonroot = 1+(nEmptyCell+nEmptyColumn)*(1+nEmptyColumn);
    //max_move_root = 1+(nEmptyCell+nEmptyColumn-1)*nEmptyColumn;
    max_move_nonroot = calc_move(nEmptyCell,nEmptyColumn);
    max_move_root = calc_move(nEmptyCell,nEmptyColumn-1);

    /* traverse each column to find out all front cards */
    memset(front_card_loc, 0, sizeof(front_card_loc));
    for (i = nEmptyColumn; i < 8; i++) {
        CARD c = TOP_CARD_OF_COLUMN(st,i);
        front_card_loc[CARD2INDEX(c)] = LOC_COLUMN+i;
    }

    /* now ready to branch children */
    n = 0;
#define ADD_MOVE(c,f,t) \
    do { \
        if (n++ < max_moves) \
            moves[n-1] = PACK_MOVE(c,f,t); \
    } while (0)

    /* traverse cards in free cell to see if we can move them */
    for (i = nEmptyCell; i < 4; i++) {
        CARD c = CARD_IN_FREECELL(st,i);
        /* try move onto another card */
        if (RANK_OF(c) < CARD_K) {
            CARD c1 = CARD_UP_ALT_1(c), c2 = CARD_UP_ALT_2(c);
            if (front_card_loc[CARD2INDEX(c1)] > 0) {
                ADD_MOVE(c, LOC_FREECELL+i, front_card_loc[CARD2INDEX(c1)]);
            }
            if (front_card_loc[CARD2INDEX(c2)] > 0) {
                ADD_MOVE(c, LOC_FREECELL+i, front_card_loc[CARD2INDEX(c2)]);
            }
        }
        /* try move to empty column */
        if (nEmptyColumn > 0) {
            ADD_MOVE(c, LOC_FREECELL+i, LOC_COLUMN+0);
        }
        /* try move to homecell */
        if (CAN_COLLECT(st,c)) {
            ADD_MOVE(c, LOC_FREECELL+i, LOC_HOMECELL);
        }
    }
    
    /* traverse cards in each column */
    for (i = nEmptyColumn; i < 8; i++) {
        CARD c = TOP_CARD_OF_COLUMN(st,i);
        int depth = 0;
        do {
            /* try move onto a front card */
            if (RANK_OF(c) < CARD_K) {
                CARD c1 = CARD_UP_ALT_1(c), c2 = CARD_UP_ALT_2(c);
                if (front_card_loc[CARD2INDEX(c1)] > 0) {
                    ADD_MOVE(c, LOC_COLUMN+i, front_card_loc[CARD2INDEX(c1)]);
                }
                if (front_card_loc[CARD2INDEX(c2)] > 0) {
                    ADD_MOVE(c, LOC_COLUMN+i, front_card_loc[CARD2INDEX(c2)]);
                }
            }
            /* try move into an empty column */
            if (nEmptyColumn > 0 && depth < max_move_root) {
                ADD_MOVE(c, LOC_COLUMN+i, LOC_COLUMN+0);
            }
            /* more options for the front card */
            if (depth == 0) {
                /* try move to homecell */
                if (CAN_COLLECT(st,c)) {
                    ADD_MOVE(c, LOC_COLUMN+i, LOC_HOMECELL);
                }
                /* try move to freecell */
                if (nEmptyCell > 0) {
                    ADD_MOVE(c, LOC_COLUMN+i, LOC_FREECELL+0);
                }
            }

            /* check the card under, if in sequence */
            if (IS_UP_ALT(CARD_UNDER(st,c),c)) {
                ++depth;
                c = CARD_UNDER(st,c);
            } else {
                break;
            }
        } while (depth < max_move_nonroot);
    }
        
    /* Spawn the children */
    /*
    if (n > max_children) {
        return -1;
    }
    for (i = 0; i < n; i++) {
        printf("Move %2d: %s\n", i+1, move2str(moves[i]));
        memmove(children+i, st, sizeof(STATE));
        MoveCard(children+i, moves[i]);
        CollectSafely(children+i);
    }
    */
    return n;
}

void DeriveState(STATE *st, const STATE *from, CARD_MOVE move)
{
    memmove(st, from, sizeof(STATE));
    MoveCard(st, move);
    CollectSafely(st);
}








#if 0

__END__

#define IS_UP_INSUIT(c1,c2) ((c1)==(c2)+4)
/* #define IS_HOMECELL_EMPTY(st,col) (IS_NONE((st)->home[col])) */

/* Collect the given card to its home cell, regardless of whether 
 * this operation is legal. 
 */
#define COLLECT_CARD(st,card) \
    do { \
        CARD c = card; \
        CARD_IN_HOMECELL(st, SUIT_OF(c)) = c; \
    } while (0)

/* Empty the specified free cell, keeping the state normalized.
 * Do not care about the card in that cell, not even if there's a card at all.
 */
#define EMPTY_FREECELL(st0,which) \
    do { \
        STATE *st = st0; \
        int j; \
        for (j = which; j > 0; j--) { \
            CARD_IN_FREECELL(st,j) = CARD_IN_FREECELL(st,j-1); \
        } \
        CARD_IN_FREECELL(st,0) = CARD_NONE; \
    } while (0)




typedef __int64 INT64;

typedef __int64 DECKMASK;
#define DECKMASK_NONE ((DECKMASK)0)
#define DECKMASK_BIT(card) ( (DECKMASK)1 << CARD2INDEX(card) )
#define DECKMASK_SET(mask,card) ((mask) |= DECKMASK_BIT(card))
#define DECKMASK_TEST(mask,card) (((mask) & DECKMASK_BIT(card)) != 0)

#define CARD_DOWN_ALT_1(card) MAKE_CARD(RANK_OF(card)-1,ALTERNATE_SUIT_1(card))
#define CARD_DOWN_ALT_2(card) MAKE_CARD(RANK_OF(card)-1,ALTERNATE_SUIT_2(card))

/* Find out all possible moves from the given state, including supermoves. 
 * Store the children in _children_. All children are normalized.
 * Return the number of children spawned, or -1 if max_children is not 
 * sufficient.
 */
int branch(const STATE *st, STATE *children, int max_children)
{
    int i, nfree, nfree2;
    int max_move_nonroot, max_move_root;

    /* supply[A].col[3] = 4 means:
     * you can move card A on to A+1,HEART, which is in column 4-1.
     */
    union _supply {
        unsigned char col[4];
        int tf;
    } supply[52];

    /* how many empty free cells are there? */
    for (nfree = 0; (nfree < 4) && IS_FREECELL_EMPTY(st,nfree); nfree++);

    /* how many empty columns are there? */
    for (nfree2 = 0; (nfree2 < 8) && IS_COLUMN_EMPTY(st,nfree2); nfree2++);
    
    /* so, how many cards can we move at a time? */
    max_move_nonroot = 1+(nfree+nfree2)*(1+nfree2);
    max_move_root = 1+(nfree+nfree2-1)*nfree2;

    /* traverse each column to check supply */
    memset(supply, 0, sizeof(supply));
    for (i = nfree2; i < 8; i++) {
        CARD c = TOP_CARD_IN_COLUMN(st,i);
        if (RANK_OF(c) > 1) {
            supply[CARD2INDEX(CARD_DOWN_ALT_1(c))][SUIT2INDEX(c)] = i+1;
            supply[CARD2INDEX(CARD_DOWN_ALT_2(c))][SUIT2INDEX(c)] = i+1;
        }
    }

    /* traverse cards in free cell to see if we can move them */
    for (i = nfree; i < 4; i++) {
        CARD c = CARD_IN_FREECELL(st,i);
        if (supply[CARD2INDEX(c)].tf) {
            /* branch this child */
            /* Move from FREECELL[i] to supply[CARD2INDEX(c)].col */

            //unsigned char *cols = supply[CARD2INDEX(c)];
            //for (j = 0; j < 4; j++)
            //EMPTY_FREECELL(st,i);
        }
        if (nfree2 > 0) {
            /* branch this child */
            /* Move from FREECELL[i] to COLUMN[nfree2-1] */
        }
    }
    
    /* traverse cards in each column */
    for (i = nfree2; i < 8; i++) {
        CARD c = TOP_CARD_IN_COLUMN(st,i);
        CARD c2;
        int j = 0;
        do {
            if (supply[CARD2INDEX(c)].tf) {
                /* branch this child */
                /* Move c from COLUMN[i] to COLUMN[supply[CARD2INDEX(c)].cols */
            }
            if (nfree2 > 0 && j < max_move_root) {
                /* branch this child */
            }
            c2 = CARD_UNDER(st,c);
            if (IS_UP_ALT(c2,c)) {
                ++j;
                c = c2;
            } else {
                break;
            }
        } while (j < max_move_nonroot);
    }
        
}

#endif