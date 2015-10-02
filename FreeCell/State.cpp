///////////////////////////////////////////////////////////////////////////
// State.cpp
//

#include "Card.h"
#include "State.h"
#include <cassert>
#include <algorithm>

namespace FreeCell
{
#if 0
	static void sort_cards(CARD cards[], int n, CARD sort2[])
	{
		int i, j;
		for (i = 0; i < n - 1; i++) {
			for (j = i + 1; j < n; j++) {
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
#endif

	void State::MoveCard(CARD card, AREA fromArea, int fromIndex, AREA toArea, int toIndex)
	{
		assert(IsCard(card));
		assert(IsSlot(fromArea, fromIndex));
		assert(IsSlot(toArea, toIndex));

		CARD topCard = card;
		switch (fromArea)
		{
		case AREA_FREECELL:
			assert(CardInFreeCell(fromIndex) == card);
			CardInFreeCell(fromIndex) = AREA_FREECELL; // CARD_NONE
			break;

		case AREA_COLUMN:
			topCard = TopCardOfColumn(fromIndex);
			TopCardOfColumn(fromIndex) = CardUnder(card);
			break;

		default:
			assert(0);
			break;
		}

		switch (toArea) 
		{
		case AREA_HOMECELL:
			assert(toIndex == SuitOf(card));
			assert(CanCollect(card));
			CardUnder(card) = AREA_HOMECELL;
			TopCardInHomeCell((Suit)toIndex) = card;
			break;

		case AREA_FREECELL:
			assert(IsFreeCellEmpty(toIndex));
			CardInFreeCell(toIndex) = card;
			CardUnder(card) = AREA_FREECELL;
			break;

		case AREA_COLUMN:
			CardUnder(card) = TopCardOfColumn(toIndex);
			TopCardOfColumn(toIndex) = topCard;
			break;

		default:
			assert(0);
			break;
		}
	}

	// A card is "safely-collectible" if and only if it is collectible
	// and all cards of its alternate color whose rank is no greater
	// than its rank minus two have been collected.
	bool State::CanCollectSafely(CARD card) const
	{
		assert(IsCard(card));

		if (!CanCollect(card))
			return false;

		if (RankOf(card) <= 2)
			return true;

		int r = RankOf(card);
		return RankOf(TopCardInHomeCell(AlternateSuit1(SuitOf(card)))) >= r - 2
			&& RankOf(TopCardInHomeCell(AlternateSuit2(SuitOf(card)))) >= r - 2;
	}

	// Collect as many cards safely as possible. This is done recursively.
	void State::CollectSafely()
	{
		int n;
		do 
		{
			n = 0;

			// Collect cards in the columns.
			for (int i = 0; i < 8; i++) 
			{
				if (!IsColumnEmpty(i))
				{
					CARD c = TopCardOfColumn(i);
					if (CanCollectSafely(c))
					{
						MoveCard(c, AREA_COLUMN, i, AREA_HOMECELL, SuitOf(c));
						++n;
					}
				}
			}

			// Collect cards in the free cells.
			for (int i = 0; i < 4; i++) 
			{
				if (!IsFreeCellEmpty(i))
				{
					CARD c = CardInFreeCell(i);
					if (CanCollectSafely(c))
					{
						MoveCard(c, AREA_FREECELL, i, AREA_HOMECELL, SuitOf(c));
						++n;
					}
				}
			}
		} while (n > 0);
	}

	// Get all possible moves from the current state.
	/* Find out all possible moves from the given state, including supermoves,
	* and store them in max_moves. Store no more than max_moves items.
	* Return the number of possible moves, which may > max_moves if max_moves
	* is not sufficient.
	*
	* CAUTION: state must be normalized before calling this function.
	*
	* There can be no more than 80 different moves from any given
	  state, calculated as follows:
	* - at most 8 cards can be moved into a free cell;
	* - at most 4 cards can be moved into a home cell (non-safely);
	* - at most 16 cards can be moved onto the top card of a non-empty column;
	* - at most 52 cards can be moved into an empty column.
	*/
	int State::GetPossibleMoves(CardMove moves[80]) const
	{
		const int numEmptyColumns = EmptyColumnCount();
		const int numEmptyFreeCells = EmptyFreeCellCount();
		const int firstEmptyColumn = FirstEmptyColumn();
		const int firstEmptyFreeCell = FirstEmptyFreeCell();
		const int maxDepthRoot = (numEmptyColumns == 0) ? 0 :
			GetMovableDepth(numEmptyFreeCells, numEmptyColumns - 1);
		const int maxDepthNonRoot = GetMovableDepth(
			numEmptyFreeCells, numEmptyColumns);

		int numMoves = 0;

		// Try move each card in the free cells.
		for (int i = 0; i < 4;i++)
		{
			if (IsFreeCellEmpty(i))
				continue;

			CARD card = CardInFreeCell(i);

			// Try move onto another card.
			for (int columnIndex = 0; columnIndex < 8; columnIndex++)
			{
				if (!IsColumnEmpty(columnIndex))
				{
					CARD topCard = TopCardOfColumn(columnIndex);
					if (IsIncrementRankAlternateColor(card, topCard))
					{
						moves[numMoves++] = CardMove(
							card, AREA_FREECELL, i, AREA_COLUMN, columnIndex);
					}
				}
			}

			// Try move into an empty column.
			if (numEmptyColumns > 0)
			{
				moves[numMoves++] = CardMove(
					card, AREA_FREECELL, i, AREA_COLUMN, firstEmptyColumn);
			}

			// Try move to home cell (non-safely).
			if (CanCollect(card))
			{
				moves[numMoves++] = CardMove(
					card, AREA_FREECELL, i, AREA_HOMECELL, SuitOf(card));
			}
		}

		// Try move cards in the columns.
		for (int srcColumn = 0; srcColumn < 8; srcColumn++)
		{
			if (IsColumnEmpty(srcColumn))
				continue;

			const CARD topCard = TopCardOfColumn(srcColumn);

			// Try move to home cell (non-safely).
			if (CanCollect(topCard))
			{
				moves[numMoves++] = CardMove(
					topCard, AREA_COLUMN, srcColumn, AREA_HOMECELL, SuitOf(topCard));
			}

			// Try move to free cell.
			if (numEmptyFreeCells > 0)
			{
				moves[numMoves++] = CardMove(
					topCard, AREA_COLUMN, srcColumn, AREA_FREECELL, firstEmptyFreeCell);
			}

			// Try move onto the top card in a non-empty column.
			if (true)
			{
				CARD card = topCard;
				for (int depth = 0; depth < maxDepthNonRoot; depth++)
				{
					// Try move onto the top card in a non-empty column.
					for (int destColumn = 0; destColumn < 8; destColumn++)
					{
						if (!IsColumnEmpty(destColumn) &&
							IsIncrementRankAlternateColor(
							card, TopCardOfColumn(destColumn)))
						{
							moves[numMoves++] = CardMove(card,
								AREA_COLUMN, srcColumn, 
								AREA_COLUMN, destColumn);
						}
					}
					if (IsIncrementRankAlternateColor(card, CardUnder(card)))
						card = CardUnder(card);
					else
						break;
				}
			}

			// Try move into an empty column.
			if (numEmptyColumns > 0)
			{				
				CARD card = topCard;
				for (int depth = 0; depth < maxDepthRoot; ++depth)
				{
					moves[numMoves++] = CardMove(card, 
						AREA_COLUMN, srcColumn, 
						AREA_COLUMN, firstEmptyColumn);

					if (IsIncrementRankAlternateColor(card, CardUnder(card)))
						card = CardUnder(card);
					else
						break;
				}
			}
		}
		return numMoves;
	}

	size_t State::GetHashCode() const
	{
		union
		{
			uint64_t qword;
			struct
			{
				uint32_t lo_dword;
				uint32_t hi_dword;
			};
			uint8_t bytes[8];
		} code;

		memcpy(&code, head, sizeof(code));
		std::sort(&code.bytes[0], &code.bytes[8]);

#if _WIN64
		static_assert(sizeof(size_t) == 8, "");
		return code.qword;
#else
		static_assert(sizeof(size_t) == 4, "");
		return code.lo_dword ^ code.hi_dword;
#endif
	}

	/*void DeriveState(STATE *st, const STATE *from, CARD_MOVE move)
	{
		memmove(st, from, sizeof(STATE));
		MoveCard(st, move);
		CollectSafely(st);
	}*/
}