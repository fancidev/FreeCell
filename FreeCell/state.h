#ifndef FCPLAY_STATE_H
#define FCPLAY_STATE_H

#include "card.h"

typedef struct STATE {

    /* The card on top of each column in the deck, or CARD_NONE
     * if that column is empty.
     * Ordered by the bottom card index in that column, which means
     * empty column comes first.
     */
    CARD head[8];

    /* The cards in hand, or CARD_NONE. 
     * Ordered by card index, which means free cell comes first.
     */
    CARD hand[4];

    /* The largest card of each suit at home, or CARD_NONE. 
     * Indexed by suit.
     */
    CARD home[4];

    /* If B = card[CARD2INDEX(A)], then B is the card under A. 
     * If there is no card under A, i.e. if A is at the bottom,
     * in the freecell, or already collected, B = CARD_NONE.
     */
    CARD next[52];

} STATE;

#define CARD_IN_FREECELL(st,which)          ((st)->hand[which])
#define CARD_IN_HOMECELL(st,suit_or_card)   ((st)->home[(unsigned)(suit_or_card)&0x3])
#define TOP_CARD_OF_COLUMN(st,col)          ((st)->head[col])
#define CARD_UNDER(st,card)                 ((st)->next[CARD2INDEX(card)])

#define IS_COLUMN_EMPTY(st,col)     IS_NONE(TOP_CARD_OF_COLUMN(st,col))
#define IS_FREECELL_EMPTY(st,col)   IS_NONE(CARD_IN_FREECELL(st,col))
#define IS_HOMECELL_EMPTY(st,suit_or_card)  (RANK_OF(CARD_IN_HOMECELL(st,suit_or_card))==0)
#define CAN_COLLECT(st,card)        IS_UP_INSUIT(card,CARD_IN_HOMECELL(st,card))

#define IS_WON(st) ( \
    RANK_OF(CARD_IN_HOMECELL(st,CARD_SPADE))==13 && \
    RANK_OF(CARD_IN_HOMECELL(st,CARD_HEART))==13 && \
    RANK_OF(CARD_IN_HOMECELL(st,CARD_CLUB))==13 && \
    RANK_OF(CARD_IN_HOMECELL(st,CARD_DIAMOND))==13 \
    )

/* Location */
typedef enum CARD_LOCATION {
    LOC_NOWHERE     = 0,
    LOC_HOMECELL    = 0x10,
    LOC_FREECELL    = 0x20,
    LOC_FREECELL_1  = LOC_FREECELL+0,
    LOC_FREECELL_2  = LOC_FREECELL+1,
    LOC_FREECELL_3  = LOC_FREECELL+2,
    LOC_FREECELL_4  = LOC_FREECELL+3,
    LOC_COLUMN      = 0x30,
    LOC_COLUMN_1    = LOC_COLUMN+0,
    LOC_COLUMN_2    = LOC_COLUMN+1,
    LOC_COLUMN_3    = LOC_COLUMN+2,
    LOC_COLUMN_4    = LOC_COLUMN+3,
    LOC_COLUMN_5    = LOC_COLUMN+4,
    LOC_COLUMN_6    = LOC_COLUMN+5,
    LOC_COLUMN_7    = LOC_COLUMN+6,
    LOC_COLUMN_8    = LOC_COLUMN+7,
} CARD_LOCATION;

#define LOCATION_TYPE(loc) ((CARD_LOCATION)(loc) & 0xf0)
#define LOCATION_INDEX(loc) ((int)(loc) & 0x0f)

typedef unsigned int CARD_MOVE;
#define PACK_MOVE(card,from,to) ( \
    ((unsigned int)(unsigned char)(CARD)(card)) | \
    ((unsigned int)(unsigned char)(CARD_LOCATION)(from) << 8) | \
    ((unsigned int)(unsigned char)(CARD_LOCATION)(to) << 16) \
    )
#define MOVE_WHAT(m) ((CARD)((unsigned)(m) & 0xff))
#define MOVE_SRC(m)  ((CARD_LOCATION)(((unsigned)(m)>>8) & 0xff))
#define MOVE_DST(m)  ((CARD_LOCATION)(((unsigned)(m)>>16) & 0xff))
/*
#define MOVE_SRC_TYPE(m)    LOCATION_TYPE(MOVE_SRC(m))
#define MOVE_SRC_INDEX(m)   LOCATION_INDEX(MOVE_SRC(m))
*/

void ResetState(STATE *st);
void NormalizeState(STATE *st);
void GetBottomCards(const STATE *st, CARD cards[8]);
void MoveCard(STATE *st, CARD_MOVE m);
// void MoveCard2(STATE *st, CARD c, CARD_LOCATION from, CARD_LOCATION to);
void DeriveState(STATE *st, const STATE *from, CARD_MOVE move);
int  CanCollectSafely(const STATE *st, CARD c);
int  CollectSafely(STATE *st);
int  FindPossibleMoves(const STATE *st, CARD_MOVE moves[], int max_moves);










#endif

#if 0

__END__

#define ALTERNATE_SUIT_1(suit_or_card) (((int)(suit_or_card)+1) & 0x3)
#define ALTERNATE_SUIT_2(suit_or_card) (((int)(suit_or_card)+3) & 0x3)

#define IS_COLUMN_EMPTY(st,col) (IS_NONE((st)->head[col]))
#define IS_FREECELL_EMPTY(st,col) (IS_NONE((st)->hand[col]))
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

#endif
