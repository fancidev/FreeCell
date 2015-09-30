#ifndef FCPLAY_CARD_H
#define FCPLAY_CARD_H

typedef enum CARD_SUIT {
    CARD_SPADE   = 0,
    CARD_HEART   = 1,
    CARD_CLUB    = 2,
    CARD_DIAMOND = 3,
} CARD_SUIT;

#define SUIT2INDEX(suit) ((int)(CARD_SUIT)(suit))
#define INDEX2SUIT(index) ((CARD_SUIT)(int)(index))

typedef enum CARD_COLOR {
    CARD_BLACK  = 0,
    CARD_RED    = 1,
} CARD_COLOR;

typedef enum CARD_RANK {
    CARD_NONE = 0,
    CARD_A = 1,
    CARD_1 = 1,
    CARD_2 = 2,
    CARD_3 = 3,
    CARD_4 = 4,
    CARD_5 = 5,
    CARD_6 = 6,
    CARD_7 = 7,
    CARD_8 = 8,
    CARD_9 = 9,
    CARD_10 = 10,
    CARD_T = 10,
    CARD_J = 11,
    CARD_Q = 12,
    CARD_K = 13,
} CARD_RANK;

#define RANK2INDEX(rank) ((int)(CARD_RANK)(rank)-1)
#define INDEX2RANK(index) ((CARD_RANK)((int)(index)+1))

typedef unsigned char CARD;
#define MAKE_CARD(rank,suit) ((CARD)(((CARD_RANK)(rank)<<2)|(CARD_SUIT)(suit)))
#define SUIT_OF(card) ((CARD_SUIT)((CARD)(card) & 0x3))
#define RANK_OF(card) ((CARD_RANK)((CARD)(card)>>2))
#define COLOR_OF(card) ((CARD_COLOR)((CARD)(card) & 0x1))
#define IS_CARD(card) ((CARD)(card) != CARD_NONE)
#define IS_NONE(card) ((CARD)(card) == CARD_NONE)
#define CARD2INDEX(card)  ((int)(CARD)(card)-4)
#define INDEX2CARD(index) ((CARD)((int)(index)+4))


/* Advanced macros */
#define ALT_SUIT_1(suit_or_card) ((CARD_SUIT)(((unsigned)(suit_or_card)+1) & 0x3))
#define ALT_SUIT_2(suit_or_card) ((CARD_SUIT)(((unsigned)(suit_or_card)+3) & 0x3))
#define CARD_UP_ALT_1(card) MAKE_CARD(RANK_OF(card)+1,ALT_SUIT_1(card))
#define CARD_UP_ALT_2(card) MAKE_CARD(RANK_OF(card)+1,ALT_SUIT_2(card))
#define CARD_UP_INSUIT(card) ((card)+4)

#define IS_UP_INSUIT(c1,c2) ((c1)==(c2)+4)
#define IS_UP_ALT(c1,c2) ((RANK_OF(c1)==RANK_OF(c2)+1) && (COLOR_OF(c1)!=COLOR_OF(c2)))


#endif
