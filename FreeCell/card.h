///////////////////////////////////////////////////////////////////////////
// Card.h
//

#pragma once

#include <cstdint>
#include <cassert>

namespace FreeCell
{
	enum Suit
	{
		SUIT_SPADE = 0,
		SUIT_HEART = 1,
		SUIT_CLUB = 2,
		SUIT_DIAMOND = 3,
	};

	enum Color
	{
		COLOR_BLACK = 0,
		COLOR_RED = 1,
	};

	enum Rank
	{
		RANK_A = 1,
		RANK_2 = 2,
		RANK_3 = 3,
		RANK_4 = 4,
		RANK_5 = 5,
		RANK_6 = 6,
		RANK_7 = 7,
		RANK_8 = 8,
		RANK_9 = 9,
		RANK_10 = 10,
		RANK_J = 11,
		RANK_Q = 12,
		RANK_K = 13,
	};

	// typedef unsigned char Card;
	typedef unsigned char CARD;
	typedef unsigned char AREA;
	typedef unsigned char SLOT;

	enum
	{
		// Special constants that represent the four home cells.
		// These constants are constructed as if they are cards
		// with rank zero.
		AREA_HOMECELL = 0,
		SLOT_HOMECELL_S = AREA_HOMECELL + 0,
		SLOT_HOMECELL_H = AREA_HOMECELL + 1,
		SLOT_HOMECELL_C = AREA_HOMECELL + 2,
		SLOT_HOMECELL_D = AREA_HOMECELL + 3,

		// Constants that represent actual cards (rank >= 1).
		CARD_AS, CARD_AH, CARD_AC, CARD_AD,
		CARD_2S, CARD_2H, CARD_2C, CARD_2D,
		CARD_3S, CARD_3H, CARD_3C, CARD_3D,
		CARD_4S, CARD_4H, CARD_4C, CARD_4D,
		CARD_5S, CARD_5H, CARD_5C, CARD_5D,
		CARD_6S, CARD_6H, CARD_6C, CARD_6D,
		CARD_7S, CARD_7H, CARD_7C, CARD_7D,
		CARD_8S, CARD_8H, CARD_8C, CARD_8D,
		CARD_9S, CARD_9H, CARD_9C, CARD_9D,
		CARD_TS, CARD_TH, CARD_TC, CARD_TD,
		CARD_JS, CARD_JH, CARD_JC, CARD_JD,
		CARD_QS, CARD_QH, CARD_QC, CARD_QD,
		CARD_KS, CARD_KH, CARD_KC, CARD_KD,

		// Constants that represent the eight columns.
		AREA_COLUMN = 0x80,
		SLOT_COLUMN_0 = AREA_COLUMN + 0,
		SLOT_COLUMN_1 = AREA_COLUMN + 1,
		SLOT_COLUMN_2 = AREA_COLUMN + 2,
		SLOT_COLUMN_3 = AREA_COLUMN + 3,
		SLOT_COLUMN_4 = AREA_COLUMN + 4,
		SLOT_COLUMN_5 = AREA_COLUMN + 5,
		SLOT_COLUMN_6 = AREA_COLUMN + 6,
		SLOT_COLUMN_7 = AREA_COLUMN + 7,

		// Constants that represent the four free cells.
		AREA_FREECELL = 0x90,
		SLOT_FREECELL_0 = AREA_FREECELL + 0,
		SLOT_FREECELL_1 = AREA_FREECELL + 1,
		SLOT_FREECELL_2 = AREA_FREECELL + 2,
		SLOT_FREECELL_3 = AREA_FREECELL + 3,

		// Bit-masks to extract AREA and index from a SLOT.
		MASK_AREA = 0xf0,
		MASK_INDEX = 0x0f,

		CARD_INVALID = 255,
		AREA_INVALID = 255,
		SLOT_INVALID = 255,
	};

	inline bool IsInvalid(CARD c)
	{
		return c == CARD_INVALID;
	}

	inline bool IsCard(CARD c)
	{
		return (c >= CARD_AS) && (c <= CARD_KD);
	}

	/*inline bool IsSlot(SLOT s)
	{
		return (s == AREA_COLUMN || s == AREA_FREECELL || s == AREA_HOMECELL);
	}*/

	inline bool IsSlot(AREA area, int index)
	{
		return (area == AREA_COLUMN && index >= 0 && index < 8)
			|| (area == AREA_FREECELL && index >= 0 && index < 4)
			|| (area == AREA_HOMECELL && index >= 0 && index < 4);
	}

	inline CARD MakeCard(Rank rank, Suit suit)
	{
		assert(rank >= 0 && rank <= 13);
		assert(suit >= 0 && suit < 4);
		return static_cast<CARD>((rank << 2) | suit);
	}

	inline Suit SuitOf(CARD card)
	{
		assert(card >= 0 && card < 56);
		return static_cast<Suit>(card & 3);
	}

	inline Rank RankOf(CARD card)
	{
		assert(card >= 0 && card < 56);
		return static_cast<Rank>(card >> 2);
	}

	//inline Color color(Card card)
	//{
	//	return static_cast<Color>(card & 0x1);
	//}

	inline Suit AlternateSuit1(Suit suit)
	{
		assert(suit >= 0 && suit < 4);
		return static_cast<Suit>(suit ^ 1);
	}

	inline Suit AlternateSuit2(Suit suit)
	{
		assert(suit >= 0 && suit < 4);
		return static_cast<Suit>(suit ^ 3);
	}

	/*inline Card change_color_1(Card card)
	{
		return static_cast<Card>(card ^ 1);
	}

	inline Card change_color_2(Card card)
	{
		return static_cast<Card>(card ^ 3);
	}*/

	inline CARD IncrementRank(CARD card)
	{
		assert(card >= 0 && card < 56);
		return static_cast<CARD>(card + 4);
	}

	inline bool IsIncrementRank(CARD c1, CARD c2)
	{
		return RankOf(c1) + 1 == RankOf(c2);
	}

	inline bool IsAlternateColor(CARD c1, CARD c2)
	{
		return (c1 ^ c2) & 1;
	}

	inline bool IsIncrementRankAlternateColor(CARD c1, CARD c2)
	{
		return IsIncrementRank(c1, c2) && IsAlternateColor(c1, c2);
	}
}