#if 0
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "card.h"
#include "state.h"
#include "cardio.h"

/* Decode a sequence of card names into cards. 
 * At most 'max_cards' elements are stored.
 * Return the number of cards decoded, or -1 on error.
 *
 * Coding format: [SHCD][0-9JQKA]
 */
int decode_cards(CARD cards[], int max_cards, const char *s)
{
    const char *s0 = s;
    int i = 0;

    while (*s) {
        CARD_SUIT suit;
        CARD_RANK rank;
        char c;

        /* skip blanks */
        while (*s && isspace(*s))
            s++;

        /* break if EOZ */
        if (*s == '\0')
            break;

        /* read suit char */
        switch (toupper(c = *s++)) {
        case 'S': suit = CARD_SPADE; break;
        case 'H': suit = CARD_HEART; break;
        case 'C': suit = CARD_CLUB; break;
        case 'D': suit = CARD_DIAMOND; break;
        default: 
            fprintf(stderr, "Unrecognized suit character '%c' at column %d.\n", c, s-s0);
            return -1;
        }

        /* read number char */
        switch (toupper(c = *s++)) {
        case 'A': rank = CARD_A; break;
        case '1': rank = CARD_A; break;
        case '2': rank = CARD_2; break;
        case '3': rank = CARD_3; break;
        case '4': rank = CARD_4; break;
        case '5': rank = CARD_5; break;
        case '6': rank = CARD_6; break;
        case '7': rank = CARD_7; break;
        case '8': rank = CARD_8; break;
        case '9': rank = CARD_9; break;
        case '0': rank = CARD_10; break;
        case 'T': rank = CARD_10; break;
        case 'J': rank = CARD_J; break;
        case 'Q': rank = CARD_Q; break;
        case 'K': rank = CARD_K; break;
        default: 
            if (c == '\0') {
                fprintf(stderr, "Missing card number at end of line.\n");
            } else {
                fprintf(stderr, "Missing card number at column %d.\n", s-s0);
            }
            return -1;
        }

        if (i >= max_cards) {
            fprintf(stderr, "Too many cards in a row.\n");
            return -1;
        }
        cards[i++] = MAKE_CARD(rank, suit);
    }

    /* return the number of cards read */
    return i;
}

const char *card2str(CARD c) 
{
    static char s[3] = {0,0,0};
    char c1, c2;

    if (RANK_OF(c) == 0)
        return "  ";

    switch (RANK_OF(c)) {
    case CARD_A: c1 = 'A'; break;
    case CARD_2: c1 = '2'; break;
    case CARD_3: c1 = '3'; break;
    case CARD_4: c1 = '4'; break;
    case CARD_5: c1 = '5'; break;
    case CARD_6: c1 = '6'; break;
    case CARD_7: c1 = '7'; break;
    case CARD_8: c1 = '8'; break;
    case CARD_9: c1 = '9'; break;
    case CARD_10: c1 = 'T'; break;
    case CARD_J: c1 = 'J'; break;
    case CARD_Q: c1 = 'Q'; break;
    case CARD_K: c1 = 'K'; break;
    default: return "??";
    }

    switch (SUIT_OF(c)) {
    case CARD_SPADE: c2 = 'S'; break;
    case CARD_HEART: c2 = 'H'; break;
    case CARD_CLUB: c2 = 'C'; break;
    case CARD_DIAMOND: c2 = 'D'; break;
    default: return "??";
    }

    s[0] = c1;
    s[1] = c2;
    return s;
}

/* Read state definitions */
int ReadState(STATE *st, const char *filename)
{
    FILE *fp;
    int i, total_n = 0;
    int mask[52];
    char buf[500];
    
    ResetState(st);
    memset(mask, 0, sizeof(mask));

    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open '%s': %s\n", filename, strerror(errno));
        return -1;
    }

    for (i = 0; i < 8; i++) {
        CARD column[7];
        int n, j;
        if (!fgets(buf, sizeof(buf), fp)) {
            fprintf(stderr, "Too few lines in data file.\n");
            fclose(fp);
            return -1;
        }
        if ((n = decode_cards(column, 7, buf)) <= 0) {
            fprintf(stderr, "Cannot decode line %d.\n", i+1);
            fclose(fp);
            return -1;
        }

        total_n += n;
        for (j = 0; j < n; j++) {
            CARD c = column[j];
            if (mask[CARD2INDEX(c)]) {
                fprintf(stderr, "Duplicate card %s in line %d item %d.\n",
                    card2str(c), i+1, j+1);
                fclose(fp);
                return -1;
            }
            mask[CARD2INDEX(c)] = 1;
        }

        TOP_CARD_OF_COLUMN(st,i) = column[n-1];
        for (j = 1; j < n; j++) {
            CARD_UNDER(st, column[j]) = column[j-1];
        }
        CARD_UNDER(st, column[0]) = CARD_NONE;
    }

    fclose(fp);
    return 0;
}

void PrintState(const STATE *st)
{
    int i, j;
    int mask[52];
    CARD columns[8][52];
    int collen[8];
    int maxlen = 0;

    printf("===============||===============\n");
    for (i = 0; i < 4; i++) {
        printf(" %s ", card2str(CARD_IN_FREECELL(st,i)));
    }
    for (i = 0; i < 4; i++) {
        printf(" %s ", card2str(CARD_IN_HOMECELL(st,i)));
    }
    printf("\n--------------------------------\n");

    memset(collen, 0, sizeof(collen));
    memset(mask, 0, sizeof(mask));
    for (i = 0; i < 8; i++) {
        int n = 0;
        CARD c;
        for (c = TOP_CARD_OF_COLUMN(st,i); IS_CARD(c); c = CARD_UNDER(st,c)) {
            if (mask[CARD2INDEX(c)]) {
                printf("**** Error: Loop at %s.\n", card2str(c));
                return;
            }
            ++mask[CARD2INDEX(c)];
            columns[i][n++] = c;
        }
        collen[i] = n;
        if (n > maxlen)
            maxlen = n;
    }

    for (j = 0; j < maxlen; j++) {
        for (i = 0; i < 8; i++) {
            if (collen[i] > 0) {
                printf(" %s ", card2str(columns[i][--collen[i]]));
            } else {
                printf("    ");
            }
        }
        printf("\n");
    }
    printf("--------------------------------\n");
    printf("  1   2   3   4   5   6   7   8\n");
    printf("--------------------------------\n");
}

const char *loc2str(CARD_LOCATION loc)
{
    const char *s;
    switch (loc) {
    case LOC_NOWHERE: s = "Nowhere"; break;
    case LOC_HOMECELL: s = "Foundation"; break;
    case LOC_FREECELL_1: s = "Freecell 1"; break;
    case LOC_FREECELL_2: s = "Freecell 2"; break;
    case LOC_FREECELL_3: s = "Freecell 3"; break;
    case LOC_FREECELL_4: s = "Freecell 4"; break;
    case LOC_COLUMN_1: s = "Column 1"; break;
    case LOC_COLUMN_2: s = "Column 2"; break;
    case LOC_COLUMN_3: s = "Column 3"; break;
    case LOC_COLUMN_4: s = "Column 4"; break;
    case LOC_COLUMN_5: s = "Column 5"; break;
    case LOC_COLUMN_6: s = "Column 6"; break;
    case LOC_COLUMN_7: s = "Column 7"; break;
    case LOC_COLUMN_8: s = "Column 8"; break;
    default: s = "Unknown"; break;
    }
    return s;
}

const char *move2str(CARD_MOVE m)
{
    static char buf[100];
    sprintf(buf, "%s from %s to %s", card2str(MOVE_WHAT(m)), 
        loc2str(MOVE_SRC(m)), loc2str(MOVE_DST(m)));
    return buf;
}
#endif