///////////////////////////////////////////////////////////////////////////
// State.h -- encodes a state of the game
//

#pragma once

#include "card.h"
#include <cassert>
#include <array>
#include <vector>

namespace FreeCell
{
	// Encodes the movement of a card.
	struct CardMove
	{
		CARD card;
		SLOT from;
		SLOT to;

		CardMove()
			: card(INVALID), from(INVALID), to(INVALID)
		{
		}

		CardMove(CARD card, AREA fromArea, int fromIndex, AREA toArea, int toIndex)
		{
			assert(IsCard(card));
			assert(IsSlot(fromArea, fromIndex));
			assert(IsSlot(toArea, toIndex));

			this->card = card;
			this->from = MakeSlot(fromArea, fromIndex);
			this->to = MakeSlot(toArea, toIndex);
		}

		CARD Card() const { return card; }

		AREA FromArea() const { return SlotArea(from); }
		
		int FromIndex() const { return SlotIndex(from); }

		AREA ToArea() const { return SlotArea(to); }

		int ToIndex() const { return SlotIndex(to); }
	};

	//__declspec(align(8)) 
	class State
	{
		// The top card in each column, or AREA_COLUMN if the column
		// is empty.
		CARD head[8];

		// The cards in the free cells, or AREA_FREECELL if the cell
		// is empty.
		CARD hand[4];

		// The largest card of each suit at home, or AREA_HOMECELL
		// if no card of that suit is at home. This array is indexed
		// by suit.
		CARD home[4];

		// Linked-list of stacked cards. This list is used to uniquely
		// identify a "distinct" state (to save from reordering of
		// columns, etc). If B = next[A - 4], then B is one of:
		//
		//   CARD_AS ... CARD_KD : if B is the card under A in a column
		//   AREA_COLUMN         : if A is the bottom card of a column
		//   AREA_FREECELL       : if A is in a free cell
		//   AREA_HOMECELL       : if A is in a home cell (collected)
		CARD next[52];

	private:

		CARD& TopCardOfColumn(int columnIndex)
		{
			assert(columnIndex >= 0 && columnIndex < 8);
			return head[columnIndex];
		}

		CARD& CardUnder(CARD card)
		{
			assert(IsCard(card));
			return next[card - 4];
		}

		CARD& TopCardInHomeCell(Suit suit)
		{
			assert(suit >= 0 && suit < 4);
			return home[suit];
		}

		CARD& CardInFreeCell(int index)
		{
			assert(index >= 0 && index < 4);
			return hand[index];
		}

	public:

		State()
		{
			memset(head, AREA_COLUMN, sizeof(head));
			memset(hand, AREA_FREECELL, sizeof(hand));
			memset(next, INVALID, sizeof(next));
			home[0] = CARD_0S;
			home[1] = CARD_0H;
			home[2] = CARD_0C;
			home[3] = CARD_0D;
		}

		CARD TopCardOfColumn(int columnIndex) const
		{
			assert(columnIndex >= 0 && columnIndex < 8);
			return head[columnIndex];
		}

		CARD CardUnder(CARD card) const
		{
			assert(IsCard(card));
			return next[card - 4];
		}

		CARD TopCardInHomeCell(Suit suit) const
		{
			assert(suit >= 0 && suit < 4);
			return home[suit];
		}

		CARD CardInFreeCell(int index) const
		{
			assert(index >= 0 && index < 4);
			return hand[index];
		}

		bool IsColumnEmpty(int columnIndex) const
		{
			assert(columnIndex >= 0 && columnIndex < 8);
			return head[columnIndex] == AREA_COLUMN;
		}

		bool IsFreeCellEmpty(int index) const
		{
			assert(index >= 0 && index < 4);
			return hand[index] == AREA_FREECELL;
		}

		bool IsHomeCellEmpty(Suit suit) const
		{
			assert(suit >= 0 && suit < 4);
			return home[suit] == suit;
		}

		int EmptyColumnCount() const
		{
			int count = 0;
			for (int columnIndex = 0; columnIndex < 8; columnIndex++)
			{
				count += IsColumnEmpty(columnIndex) ? 1 : 0;
			}
			return count;
		}

		int FirstEmptyColumn() const
		{
			for (int i = 0; i < 8; i++)
			{
				if (IsColumnEmpty(i))
					return i;
			}
			return -1;
		}

		int EmptyFreeCellCount() const
		{
			int count = 0;
			for (int index = 0; index < 4; index++)
			{
				count += IsFreeCellEmpty(index) ? 1 : 0;
			}
			return count;
		}

		int FirstEmptyFreeCell() const
		{
			for (int i = 0; i < 4; i++)
			{
				if (IsFreeCellEmpty(i))
					return i;
			}
			return -1;
		}

		bool IsWinning() const
		{
			return RankOf(home[0]) == 13
				&& RankOf(home[1]) == 13
				&& RankOf(home[2]) == 13
				&& RankOf(home[3]) == 13;
		}

		int NumberOfCardsCollected() const
		{
			return RankOf(home[0]) + RankOf(home[1]) + RankOf(home[2]) + RankOf(home[3]);
		}

		bool CanCollect(CARD card) const
		{
			assert(IsCard(card));
			return card == IncrementRank(TopCardInHomeCell(SuitOf(card)));
		}

		bool CanCollectSafely(CARD card) const;

		int CollectSafely();

		/*Card GetBottomCard(ColumnIndex column) const
		{
		Card bottomCard;
		Card card = GetTopCard(column);
		while (card != SLOT_COLUMN)
		{
		bottomCard = card;
		card = GetCardUnder(card);
		}
		return bottomCard;
		}
		*/
		//void SortColumns();

		void MoveCard(CARD card, AREA fromArea, int fromIndex, AREA toArea, int toIndex);

		int GetPossibleMoves(CardMove moves[80]) const;

		size_t GetHashCode() const;

		// Returns true if this state is equivalent to another state,
		// subject to reordering of columns and cards in free cells.
		bool IsEquivalentTo(const State &other) const
		{
			return memcmp(next, other.next, sizeof(next)) == 0;
		}

		void PlaceCardInColumn(CARD card, int columnIndex)
		{
			assert(IsCard(card));
			assert(columnIndex >= 0 && columnIndex < 8);
			assert(CardUnder(card) == INVALID);
			CardUnder(card) = TopCardOfColumn(columnIndex);
			TopCardOfColumn(columnIndex) = card;
		}
	};

	inline int GetMovableDepth(int numEmptyCells, int numEmptyColumns)
	{
		int n = numEmptyCells;
		int m = numEmptyColumns;
		return n * (m + 1) + 1 + m * (m + 1) / 2;
	}
}