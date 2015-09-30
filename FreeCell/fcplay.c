#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "card.h"
#include "cardio.h"
#include "state.h"
#include "statemap.h"
#include "gengame.h"


/*
typedef __int64 INT64;
typedef __int64 DECKMASK;
#define DECKMASK_NONE ((DECKMASK)0)
#define DECKMASK_BIT(card) ( (DECKMASK)1 << CARD2INDEX(card) )
#define DECKMASK_SET(mask,card) ((mask) |= DECKMASK_BIT(card))
#define DECKMASK_TEST(mask,card) (((mask) & DECKMASK_BIT(card)) != 0)
*/


/*
#define CARD_DOWN_ALT_1(card) MAKE_CARD(RANK_OF(card)-1,ALTERNATE_SUIT_1(card))
#define CARD_DOWN_ALT_2(card) MAKE_CARD(RANK_OF(card)-1,ALTERNATE_SUIT_2(card))
*/

/* Compute score for the given state. Smaller is better. May be negative. */
int compute_score1(const STATE *st)
{
    int cell_punish[5] = { 0, 1, 2, 4, 7 };
    int score = 0;
    int nEmptyCell, nEmptyColumn;
    int i;

    /* how many empty cells */
    for (i = 0; (i < 4) && IS_FREECELL_EMPTY(st,i); i++);
    nEmptyCell = i;
    score += 10*cell_punish[4-nEmptyCell];

    /* how many empty columns */
    for (i = 0; (i < 8) && IS_COLUMN_EMPTY(st,i); i++);
    nEmptyColumn = i;
    score += 5*(8-nEmptyColumn);

    /* Go through each column. An empty column earns zero. 
     */
    for (i = nEmptyColumn; i < 8; i++) {
        int depth = 1, cost = 1, pts = 1;
        CARD c = TOP_CARD_OF_COLUMN(st,i);
        CARD prevCard = CARD_NONE;
        for (c = CARD_UNDER(st,c); IS_CARD(c); c = CARD_UNDER(st,c)) {
            if (!IS_UP_ALT(c, prevCard)) {
                cost++;
            }
            pts += cost;
            prevCard = c;
            depth++;
        }
        score += pts;
    }

    /* punish empty homecell */
    for (i = 0; i < 4; i++) {
        if (IS_HOMECELL_EMPTY(st, INDEX2SUIT(i))) 
            score += 20;
    }

    return score;
}

int compute_score2(const STATE *st)
{
    int cell_punish[5] = { 0, 1, 2, 4, 7 };
    int score = 0;
    int nEmptyCell, nEmptyColumn;
    int i;

    /* how many empty cells */
    for (i = 0; (i < 4) && IS_FREECELL_EMPTY(st,i); i++);
    nEmptyCell = i;
    score += 10*cell_punish[4-nEmptyCell];

    /* how many empty columns */
    for (i = 0; (i < 8) && IS_COLUMN_EMPTY(st,i); i++);
    nEmptyColumn = i;
    score += 5*(8-nEmptyColumn);

    /* Go through each column. An empty column earns zero. 
     */
    for (i = nEmptyColumn; i < 8; i++) {
        int depth = 1, cost = 1, pts = 1;
        CARD c = TOP_CARD_OF_COLUMN(st,i);
        CARD prevCard = CARD_NONE;
        for (c = CARD_UNDER(st,c); IS_CARD(c); c = CARD_UNDER(st,c)) {
            depth++;
            if (!IS_UP_ALT(c, prevCard)) {
                cost = depth;
            }
            pts += cost;
            prevCard = c;
        }
        score += pts;
    }

    /* punish empty homecell */
    for (i = 0; i < 4; i++) {
        if (IS_HOMECELL_EMPTY(st, INDEX2SUIT(i))) 
            score += 20;
    }

    return score;
}

static int strategy = 0;

int compute_score3(const STATE *st);
int compute_score4(const STATE *st);

/* IMPORTANT: A won state MUST have score=0. Any other state MUST have score > 0. */
int compute_score(const STATE *st)
{
    switch (strategy) {
    case 1:
        return compute_score1(st);
    case 2:
        return compute_score2(st);
    case 3:
        return compute_score3(st);
    case 4:
        return compute_score4(st);
    case 13:
        return compute_score1(st)+compute_score3(st);
    default:
        printf("No strategy!\n");
        abort();
        return 0;
    }
}


/* Expand _level_ levels of offspring from the _parent_-th state in _map_.
 * Return the position of the won state if any.
 */
static STATEMAPPOS expand_children(STATEMAP *map, STATEMAPPOS parent, int level)
{
    int gen, nparent;

    /* expand each generation ... */
    nparent = 1;
    for (gen = 1; gen <= level; gen++) {

        int nchild = 0; /* total population in this generation */

        /* expand each parent in (gen-1)-th generation */
        while (nparent-- > 0) {
            CARD_MOVE moves[80];
            const STATE *st;
            int i, n;

            /* spawn children from the parent state */
            if (gen > 1) {
                parent = NextState(map);
            }
            st = GetState(map, parent);
            n = FindPossibleMoves(st, moves, 80);
            assert(n <= 80);

            /* insert each child into the state map, if not exists */
            for (i = 0; i < n; i++) {
                STATE child;
                DeriveState(&child, st, moves[i]);
                if (!LookupStateMap(map, &child)) {
                    int score = (gen == level)? compute_score(&child) : (gen-level);
                    STATEMAPPOS pos = AddState(map, &child, parent, moves[i], score);
                    if (IS_WON(&child)) {
                        return pos;
                    }
                    nchild++;
                }
            }
        }
        nparent = nchild;
    }
    assert(map->score_map->min_key >= 0);

    return STATEMAPPOS_NULL;
}

typedef struct SOLVER_INFO {
    int nsteps;
    int nodes_expanded;
    int nodes_fresh;
} SOLVER_INFO;

/* Try solve the position st0 */
void solve(STATE *st0, SOLVER_INFO *info)
{
    const STATE *st;
    STATEMAPPOS current, winpos = 0;
    STATEMAP *map = CreateStateMap();

    /* normalize state */
    NormalizeState(st0);
    CollectSafely(st0);

    /* add the initial position to the map */
    AddState(map, st0, 0, 0, 0);

    /* spawn each state in order of their score */
    while (current = NextState(map)) {
        st = GetState(map, current);
        if (IS_WON(st)) {
            winpos = current;
            break;
        }

        /* expand 2 levels of offspring */
        winpos = expand_children(map, current, 1);
        if (winpos) {
            break;
        }
        
        /* put a limit on expansion */
        if (StateMapSize(map) > 100000) {
        // if (StateMapSize(map) - map->nfresh > 1000) {
            printf("Too many states expanded: %d\n", StateMapSize(map));
            current = NextState(map);
            st = GetState(map, current);
            printf("Next node to expand: \n");
            PrintState(st);
            current = 0;
            break;
        }
        //printf(">>> Press ENTER for next loop...\n");
        //scanf("%s", buf);
    }

    /* now expansion finished... */
    if (winpos) {
        info->nsteps = GetDepth(map, winpos);
        //printf("******** WON (%d steps) ********\n", GetDepth(map, winpos));
    } else {
        info->nsteps = -1;
        //printf("No solution.\n");
    }

    /* print solution and statistics */
    // PrintStateMap(map);
    if (0) {
        int steps[1000];
        int i, n;

        current = winpos;
        for (i = 0; i < 1000; i++) {
            steps[i] = current;
            current = GetParentState(map, current);
            if (current == 0) 
                break;
        }
        if (i == 1000) {
            printf("Too many steps (>1000)\n");
        } else {
            n = i;
            printf("Initial state:\n");
            PrintState(GetState(map, steps[n]));
            for (i = n-1; i >= 0; i--) {
                current = steps[i];
                printf("Move #%d (%d): %s\n", n-i, 
                    compute_score(GetState(map, current)),
                    move2str(GetMove(map, current)));
                PrintState(GetState(map, current));
            }
        }
    }

    info->nodes_fresh = map->nfresh;
    info->nodes_expanded = StateMapSize(map) - map->nfresh;
    DestroyStateMap(map);
}

int compute_score3(const STATE *st)
{
    int cell_punish[5] = { 0, 1, 2, 4, 6 };
    //int col_punish[9] = { 0,   1, 2, 3, 4,   5, 6, 8, 10 };
    int col_punish[9] = { 0,   1, 2, 3, 4,   5, 6, 7, 8 };
    int score = 0;
    int nEmptyCell, nEmptyColumn;
    int i;
    unsigned char card_depth[52], card_cost[52];
    int chaos;
    //int spaces;

    /* how many empty cells */
    for (i = 0; (i < 4) && IS_FREECELL_EMPTY(st,i); i++);
    nEmptyCell = i;
    score += 10*cell_punish[4-nEmptyCell];

    /* how many empty columns */
    for (i = 0; (i < 8) && IS_COLUMN_EMPTY(st,i); i++);
    nEmptyColumn = i;
    score += 10*col_punish[8-nEmptyColumn];

//#define calc_move(n1,n2) ((n1)*((n2)+1)+1+(n2)*((n2)+1)/2)
//    spaces = calc_move(nEmptyCell, nEmptyColumn-1);
//    if (spaces > 12) spaces = 12;
//    score += (13 - spaces)*5;

    /* Go through each column. An empty column earns zero. 
     */
    memset(card_depth, 0, sizeof(card_depth));
    memset(card_cost, 0, sizeof(card_cost));
    for (i = nEmptyColumn; i < 8; i++) {
        int depth = 1, cost = 1, pts = 1;
        CARD c = TOP_CARD_OF_COLUMN(st,i);
        CARD prevCard = CARD_NONE;
        card_depth[CARD2INDEX(c)] = 1;
        for (c = CARD_UNDER(st,c); IS_CARD(c); c = CARD_UNDER(st,c)) {
            depth++;
            if (!IS_UP_ALT(c, prevCard)) {
                cost = depth;
            }
            card_depth[CARD2INDEX(c)] = depth;
            card_cost[CARD2INDEX(c)] = cost;
            pts += cost;
            // pts += cost*(4-(RANK_OF(c)+2)/4);
            prevCard = c;
        }
        score += pts;
        /* punish bottom card if not K */
        // score += (13 - RANK_OF(prevCard));
    }

    /* punish for out-of-order cards */
    /* The following is two-sided chaos */
    /*
    chaos = 0;
    for (i = 0; i < 4; i++) {
        CARD c = CARD_UP_INSUIT(CARD_IN_HOMECELL(st,INDEX2SUIT(i)));
        for ( ; RANK_OF(c) <= 13; c = CARD_UP_INSUIT(c)) {
            chaos += abs((int)card_depth[CARD2INDEX(c)] - RANK_OF(c));
        }
    }
    score += chaos * 2/3;
    */
    /* The following is one-sided chao, which seems to work better */
    chaos = 0;
    for (i = 0; i < 4; i++) {
        CARD c = CARD_UP_INSUIT(CARD_IN_HOMECELL(st,INDEX2SUIT(i)));
        for ( ; RANK_OF(c) <= 13; c = CARD_UP_INSUIT(c)) {
            if (card_depth[CARD2INDEX(c)] > RANK_OF(c)-1) 
                chaos += card_depth[CARD2INDEX(c)] - RANK_OF(c) + 1;
        }
    }
    score += chaos;
    
    /* compare each pair of cards to see if they're in order. */
    


    /* punish empty homecell */
    for (i = 0; i < 4; i++) {
        if (IS_HOMECELL_EMPTY(st, INDEX2SUIT(i))) 
            score += 30;
    }

    return score;
}

int compute_score4(const STATE *st)
{
    int cell_punish[5] = { 0, 1, 2, 4, 6 };
    //int col_punish[9] = { 0,   1, 2, 3, 4,   5, 6, 8, 10 };
    int col_punish[9] = { 0,   1, 2, 3, 4,   5, 6, 7, 8 };
    int score = 0;
    int nEmptyCell, nEmptyColumn;
    //int nEmptyHome;
    CARD c;
    int i;
    char col_depth[8];      /* number of cards in each column */
    char card_col[52];      /* 0-7=tableau, -1=freecell, -2=homecell */
    char card_depth[52];    /* 1=top */
    //char card_cost[52];
    //int chaos;
    //int spaces;

    /* how many empty cells */
    for (i = 0; (i < 4) && IS_FREECELL_EMPTY(st,i); i++);
    nEmptyCell = i;
    score += 5*cell_punish[4-nEmptyCell];

    /* how many empty columns */
    for (i = 0; (i < 8) && IS_COLUMN_EMPTY(st,i); i++);
    nEmptyColumn = i;
    // score += 10*col_punish[8-nEmptyColumn];

//#define calc_move(n1,n2) ((n1)*((n2)+1)+1+(n2)*((n2)+1)/2)
//    spaces = calc_move(nEmptyCell, nEmptyColumn-1);
//    if (spaces > 12) spaces = 12;
//    score += (13 - spaces)*5;

    /* Go through each column. An empty column earns zero. 
     */

    /* Find out the positions of cards */
    nEmptyColumn = 0;
    for (i = 0; i < 8; i++) {
        int depth = 0;
        CARD c;
        for (c = TOP_CARD_OF_COLUMN(st,i); IS_CARD(c); c = CARD_UNDER(st,c)) {
            card_col[CARD2INDEX(c)] = i;
            card_depth[CARD2INDEX(c)] = ++depth;
        }
        if (depth == 0) {
            ++nEmptyColumn;
        }
        col_depth[i] = depth;
    }
    nEmptyCell = 0;
    for (i = 0; i < 4; i++) {
        c = CARD_IN_FREECELL(st,i);
        if (IS_CARD(c)) {
            card_col[CARD2INDEX(c)] = -1;
            card_depth[CARD2INDEX(c)] = 0;
        } else {
            ++nEmptyCell;
        }
    }
    //nEmptyHome = 0;
    for (i = 0; i < 4; i++) {
        CARD c = CARD_IN_HOMECELL(st,INDEX2SUIT(i));
        for ( ; IS_CARD(c); c = CARD_DOWN_INSUIT(c)) {
            card_col[CARD2INDEX(c)] = -2;
            card_depth[CARD2INDEX(c)] = 0;
        }
    }

    /* Traverse each rank pair to check their minimal cost. That is,
     * We check red-Q/black-Q against black-K/red-K, J against Q, etc.
     * For K, check against column bottom; for A, check against homecell.
     *
     * For example, when we check 8-Club/Spade with 9-Heart/Diamond,
     * we have the following four costs:
     *  c_11 - cost of moving club to heart
     *  c_12 - cost of moving club to diamond
     *  c_21 - cost of moving spade to heart
     *  c_22 - cost of moving spade to diamond
     * And we choose to minimize (c_11+c_22, c_12+c_21).
     *
     * If, say, 9-Heart is already collected to homecell. Then c_11
     * becomes the cost of collecting 8-club to homecell, which includes
     * the cost of collecting A-club through 7-club.
     *
     * The cost of moving card A to card B, is depth[A]+depth[B] if 
     * they are in different columns, or max(depth[A],depth[B]) if 
     * they are in the same column.
     *
     * The cost of moving card A in freecell to card B in tableau 
     * is depth[B].
     * 
     * The cost of moving card A in tableau to card B in freecell
     * is cost[B] + depth[A].
     */

    /* Compute the cost of collecting to homecell */
    for (i = 0; i < 4; i++) {
        CARD_RANK r = RANK_OF(CARD_IN_HOMECELL(st,INDEX2SUIT(i)));
        for (
TBD

        for ( ; IS_CARD(c); c = CARD_DOWN_INSUIT(c)) {
            card_col[CARD2INDEX(c)] = -2;
            card_depth[CARD2INDEX(c)] = 0;
        }
    }

    /* Traverse each column, and punish out-of-order card pairs */
    for (i = nEmptyColumn; i < 8; i++) {
        int depth = 0, cost = 1, pts = 0;
        int j, k;
        CARD prevCard = CARD_NONE;
        CARD cards[52];
        int ccost[52];

        for (c = TOP_CARD_OF_COLUMN(st,i); IS_CARD(c); c = CARD_UNDER(st,c)) {
            if (RANK_OF(c) < RANK_OF(prevCard)) {
                ++cost;
            }
            ccost[depth] = cost;
            //ccost[depth] = depth;
            cards[depth] = c;
            ++depth;
            prevCard = c;
        }

        for (j = 0; j < depth-1; j++) {
            for (k = j+1; k < depth; k++) {
                if (RANK_OF(cards[j]) > RANK_OF(cards[k])) {
                    pts += ccost[k]-ccost[j];
                }
            }
        }

        score += pts;
    }


    /* punish empty homecell */
    /*
    for (i = 0; i < 4; i++) {
        if (IS_HOMECELL_EMPTY(st, INDEX2SUIT(i))) 
            score += 30;
    }
    */

    return score;
}


int main(int argc, char *argv[])
{
    STATE *st = (STATE *)malloc(sizeof(STATE));
    int i, n;
    int gamenumber;
    CARD_MOVE moves[80];
    SOLVER_INFO si, si_tot;

    int difficult_ones[] = {
        169, 178, 258, 454, 617, 718, 1689, 1941, 2021, 2350, 2577, 
        2607, 2670, 2772, 3285, 3342, 3349, 3685, 3772, 3788, 3801, 
        3973, 4257, 4368, 4540, 4591, 4714, 4946, 5179, 5374, 5453, 
        5482, 5490, 5548, 5557, 6343, 6673, 6745, 6751, 6768, 7107, 
        7160, 7600, 7700, 8005, 8323, 8534, 8591, 8652, 8678, 8749, 
        8820, 9250, 9385, 9538, 9617, 9700, 10589, 11281, 11386, 11409, 
        11430, 11677, 11854, 12211, 12313, 13015, 14051, 14188, 14676, 
        14795, 14879, 14965, 14977, 15023, 15099, 15130, 15133, 15164, 
        15227, 15238, 15710, 15746, 15905, 15939, 16191, 16575, 16576, 
        17277, 17524, 17764, 17768, 18623, 18992, 19410, 19484, 19633, 
        19763, 19861, 20055, 20251, 20589, 20715, 20912, 21051, 21185, 
        21278, 21785, 21896, 21899, 22332, 24063, 24457, 24549, 24735, 
        25123, 25155, 25450, 25599, 25602, 25790, 25856, 25995, 26093, 
        26183, 26197, 26369, 26421, 26576, 26693, 26694, 26710, 27188, 
        29001, 29154, 29198, 29345, 29704, 30000, 30108, 30394, 30615, 
        30712, 30801, 31044, 31266, 31465, 31601, 31647, 31729, 31918, 
        31945, 0
    };

    int games[] = {
        165, 495, 857, 740, 705, 932, 792, 253, 390, 433, 0,
        739671, /* easy     (expanded=   4,815/  2,922) */
        15893,  /* easy     (expanded=   2,216/ 14,616) */
        617,    /* easy     (expanded=   7,656/ 25,281) */
        10692,  /* hard     (expanded=  35,298/ 52,142) */
        35254,  /* extreme  (expanded= 134,104/ 20,963) */
        57148,  /* extreme  (expanded= 371,421/307,025) */
        11982,  /* unsolvable (expaned=61,643/0) */
        1941,   /* hard     (expanded=  33,307/ 31,266) */
        0 };

    
    /*
    if (read_state(st, "D:\\15893.txt") < 0) {
        return 1;
    }
    printf("Initial State:\n");
    print_state(st);
    */

    //GenerateGame(st, gamenumber);
    //printf("Game #%d:\n", gamenumber);
    //PrintState(st);

    /*
    printf("Normalized State:\n");
    NormalizeState(st);
    PrintState(st);

    n = CollectSafely(st);
    printf("\nNumber of cards auto collected = %d\n", n);
    PrintState(st);
    */

    /*
    strategy 1: (Aggressive)
      General: (fail=187,223,383)
            
        1-100:      83       883      6016      6900
        1-200:      77      1940      9625     11565
        1-500:      73      2474     13848     16322
                    74      1720      8634     10355
      Hard ones:
        Avg.        54    176151    150873    327024

    strategy 2: (Benchmark)
      General: (fail=165)
  Avg.   499>       66      1525      7097      8622  ( 165 failed )
      Hard ones:
        Avg.        85     81307     56776    138084

    strategy 3: (Conservative)
      General (1-500):
  Avg.   499>       70      1359      6420      7780  ( 495 failed )
  Avg.   500>       71      1857      9535     11393  ( all solved; punish bottom card )
* Avg.   500>       63      1238      5410      6648  ( all solved; chaos weight  .5 )
  Avg.   500>       63      1521      6461      7983  ( all solved; chaos weight 1.0 )
  Avg.   500>       63      1027      4921      5948  ( all solved; chaos weight  .67 )

  Avg.   499>       55      1476      5763      7239  ( 495 failed; chaos .5 )
  Avg.   499>       55      1400      5160      6560  ( 440 failed; chaos .67 )
  Avg.   500>       54      1744      7906      9650  ( all solved; chaos 1.0 )
  Avg.   499>       57      1354      3369      4723  ( 495 failed; one-sided chaos )

      1-1000:
  Avg.   997>       69      1603      8016      9619  ( failed 857, 740, 705 )
  Avg.   999>       63      1876      5231      7107  ( failed 932; chao .5 )
  Avg.   998>       63      1781      4684      6465  ( failed 792, 932; chao .67 )

      Hard ones:
        Avg.        80     83504     16759    100264

    Strategy 4: (1-500)
  Avg.   498>       57      2784      5564      8349  ( failed 282, 495 )


  [limit expand<=100,000]
  S1: Avg.   483>       55       814      4058      4872
  S2: Avg.   498>       57       913      4570      5483
  S3: Avg.   498>       57       935      3103      4039
  S4: Avg.   496>       55       752      2801      3553
      Avg.   494>       61      1940      1694      3635
      Avg.   497>       64      3005       885      3890
      Avg.   495>       63      2825      1247      4073

  So:
    - try fix chao method.
    - try compare pairs.

    */
    strategy = 4;

    printf("      Game     Steps  Expanded     Fresh     Total\n");
    printf("----------------------------------------------------\n");

    si_tot.nodes_expanded = 0;
    si_tot.nodes_fresh = 0;
    si_tot.nsteps = 0;
    n = 0;

#if 0
    for (i = 0; gamenumber = games[i]; i++) 
#elif 1
    for (gamenumber = 1; gamenumber <= 500; gamenumber++) 
#else
    gamenumber = 932;
#endif
    {
        GenerateGame(st, gamenumber);
        solve(st, &si);

        printf("%10d  %8d  %8d  %8d  %8d\n", gamenumber,
            si.nsteps, si.nodes_expanded, si.nodes_fresh,
            si.nodes_expanded+si.nodes_fresh);

        if (si.nsteps > 0) {
            si_tot.nodes_expanded += si.nodes_expanded;
            si_tot.nodes_fresh += si.nodes_fresh;
            si_tot.nsteps += si.nsteps;
            n++;
        }
    }
    if (n == 0)
        n = 1;
    printf("----------------------------------------------------\n");
    printf("Avg.%6d> %8d  %8d  %8d  %8d\n", n,
        si_tot.nsteps/n, si_tot.nodes_expanded/n, si_tot.nodes_fresh/n,
        (si_tot.nodes_expanded+si_tot.nodes_fresh)/n);

    return 0;

    n = FindPossibleMoves(st, moves, 80);
    printf("\nPossible Moves = %d\n", n);
    for (i = 0; i < n; i++) {
        STATE st2;
        printf("**** Move %2d: %s\n", i+1, move2str(moves[i]));
        DeriveState(&st2, st, moves[i]);
        PrintState(&st2);
    }

    //printf("\n\n\nBranching this state:\n-----------------------------\n");
    //n = branch(st, list, 100);
    //printf("\nNumber of children = %d\n", n);

    return 0;
}
