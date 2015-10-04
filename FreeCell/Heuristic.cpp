///////////////////////////////////////////////////////////////////////////
// Heuristic.cpp

#include "Heuristic.h"
#include <cassert>

namespace FreeCell
{
	static size_t ComputeHeuristic1(const State &state)
	{
		size_t score = 0;

		//int cell_punish[5] = { 0, 1, 2, 4, 7 };

		//int nEmptyCell, nEmptyColumn;
		//int i;

		///* how many empty cells */
		//for (i = 0; (i < 4) && IS_FREECELL_EMPTY(st, i); i++);
		//nEmptyCell = i;
		//score += 10 * cell_punish[4 - nEmptyCell];

		///* how many empty columns */
		//for (i = 0; (i < 8) && IS_COLUMN_EMPTY(st, i); i++);
		//nEmptyColumn = i;
		//score += 5 * (8 - nEmptyColumn);

		// Go through each column.
		for (int columnIndex = 0; columnIndex < 8; columnIndex++)
		{
			//int depth = 1, cost = 1, pts = 1;
			CARD card = state.TopCardOfColumn(columnIndex);
			while (IsCard(card))
			{
				CARD c = card;
				while (IsCard(c = state.CardUnder(c)))
				{
					if (RankOf(c) <= RankOf(card))
					{
						++score;
					}
				}
				card = state.CardUnder(card);
			}
		}

		///* punish empty homecell */
		//for (i = 0; i < 4; i++) {
		//	if (IS_HOMECELL_EMPTY(st, INDEX2SUIT(i)))
		//		score += 20;
		//}

		return score;
	}

	static size_t ComputeHeuristic2(const State &state)
	{
		size_t score = 0;

		//int cell_punish[5] = { 0, 1, 2, 4, 7 };

		//int nEmptyCell, nEmptyColumn;
		//int i;

		///* how many empty cells */
		//for (i = 0; (i < 4) && IS_FREECELL_EMPTY(st, i); i++);
		//nEmptyCell = i;
		//score += 10 * cell_punish[4 - nEmptyCell];

		///* how many empty columns */
		//for (i = 0; (i < 8) && IS_COLUMN_EMPTY(st, i); i++);
		//nEmptyColumn = i;
		//score += 5 * (8 - nEmptyColumn);

		// Go through each column.
		for (int columnIndex = 0; columnIndex < 8; columnIndex++)
		{
			CARD card = state.TopCardOfColumn(columnIndex);
			if (IsCard(card))
			{
				size_t cost = 0; // or 0 ?
				CARD c = card;
				for (CARD c2; IsCard(c2 = state.CardUnder(c)); c = c2)
				{
					if (!IsIncrementRankAlternateColor(c, c2))
					{
						++cost;
					}
					score += cost;
				}
			}
		}

		///* punish empty homecell */
		//for (i = 0; i < 4; i++) {
		//	if (IS_HOMECELL_EMPTY(st, INDEX2SUIT(i)))
		//		score += 20;
		//}

		return score;
	}

	// Same as H1 but additionally penalize occupied free cells and
	// occupied columns. Expands more states than H1 but solves more
	// problems than H1 more quickly.
	static size_t ComputeHeuristic3(const State &state)
	{
		return ComputeHeuristic1(state)
			+ (4 - state.EmptyFreeCellCount()) * 10
			+ (8 - state.EmptyColumnCount()) * 10;
	}

	// Same as H2 but additionally penalize occupied free cells and
	// occupied columns. Expands fewer states than H2 and solves more
	// problems than H2 more quickly.
	static size_t ComputeHeuristic4(const State &state)
	{
		return ComputeHeuristic2(state)
			+ (4 - state.EmptyFreeCellCount()) * 10
			+ (8 - state.EmptyColumnCount()) * 10;
	}

	// We compute the lower bound by checking each outstanding card
	// in turn. Relax the rules by assuming there are infinite empty
	// columns, and you can move as many cards as you like as long 
	// as they are increasing-rank-alternate-color. How many moves
	// are needed?
	// 
	static size_t ComputeHeuristic5(const State &state)
	{
		return ComputeHeuristic4(state);
		//+ (13-RankOf( (state.TopCardInHomeCell(SUIT_CLUB)))
		//+ (4 - state.EmptyFreeCellCount()) * 10
		//+ (8 - state.EmptyColumnCount()) * 10;
	}

	//
	// The heuristics are compared using the first 100 games and
	// setting a limit numStatesProcessed to 10000. The quality
	// of the heuristics are:
	//
	//                        H1      H2      H3      H4
	// Summary -----------------------------------------
	//   OK                   93      88      99     100
	//   NS                    0       0       0       0
	//   FL                    7      12       1       0
	// When OK -----------------------------------------
	//   Avg # Moves          47      49      47      48
	//   Avg # Processed    1564    1994    1030     677
	//   Avg # Expanded     6100   16151   16480   16113
	//
	// Conclusion: H4 > H3 > H1 > H2.
	//
	size_t ComputeHeuristic(const State &state, int heuristic)
	{
		switch (heuristic)
		{
		case 1:
			return ComputeHeuristic1(state);
		case 2:
			return ComputeHeuristic2(state);
		case 3:
			return ComputeHeuristic3(state);
		case 0:
		case 4:
			return ComputeHeuristic4(state);
		case 5:
			return ComputeHeuristic5(state);
		default:
			assert(0);
			return 0;
		}
	}
}
