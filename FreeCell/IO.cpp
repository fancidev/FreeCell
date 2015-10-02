///////////////////////////////////////////////////////////////////////////
// IO.cpp -- input/output routines

#include <iostream>
#include <string>
#include <algorithm>

#include "Card.h"
#include "State.h"
#include "IO.h"

namespace FreeCell
{
	static bool TryParseRank(char c, Rank *result)
	{
		const char *s = "A23456789TJQK";
		const char *p = strchr(s, c);
		if (p == nullptr)
		{
			*result = static_cast<Rank>(0);
			return false;
		}
		else
		{
			*result = static_cast<Rank>(p - s + 1);
			return true;
		}
	}

	static bool TryParseSuit(char c, Suit *suit)
	{
		switch (c)
		{
		case 'S':
			*suit = SUIT_SPADE;
			return true;
		case 'H':
			*suit = SUIT_HEART;
			return true;
		case 'C':
			*suit = SUIT_CLUB;
			return true;
		case 'D':
			*suit = SUIT_DIAMOND;
			return true;
		default:
			*suit = static_cast<Suit>(0);
			return false;
		}
	}

	// Reads a state from a stream.
	// The format must strictly follows:
	//
	// 8C 8D AS 4H    3H 5S 6C
	// TC 8D AS 4H    3H 5S 6C
	// 3D 8D AS       3H 5S 6C
	// XX XX
	//    XX
	//    YY
	//
	std::istream& operator >> (std::istream &stream, State &state)
	{
		int numCardsRead = 0;

		while (true)
		{
			std::string line;
			std::getline(stream, line);
			if (line.size() == 0) // blank line
				break;
			if (line[0] == '#') // comment
				continue;

			if (line.size() % 3 != 2)
				goto failure;
			if (line.size() >= 8 * 3)
				goto failure;

			line.push_back(' ');
			size_t columnCount = line.size() / 3;
			for (size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex)
			{
				if (line[columnIndex * 3 + 2] != ' ')
					goto failure;
				if (line[columnIndex * 3 + 0] == ' ' &&
					line[columnIndex * 3 + 1] == ' ')
					continue;

				Rank rank;
				Suit suit;
				if (!TryParseRank(line[columnIndex * 3 + 0], &rank) ||
					!TryParseSuit(line[columnIndex * 3 + 1], &suit))
					goto failure;

				CARD card = MakeCard(rank, suit);
				if ((const_cast<const State&>(state)).CardUnder(card) != INVALID)
					goto failure;

				state.PlaceCardInColumn(card, static_cast<int>(columnIndex));
				++numCardsRead;
			}
		}
		if (numCardsRead != 52)
			goto failure;

		return stream;

	failure:
		stream.setstate(std::ios::failbit);
		return stream;
	}

	static char FormatRank(Rank rank)
	{
		if (rank >= 1 && rank <= 13)
		{
			const char *s = "A23456789TJQK";
			return s[rank - 1];
		}
		return '\0';
	}

	static char FormatSuit(Suit suit)
	{
		switch (suit)
		{
		case SUIT_SPADE:
			return 'S';
		case SUIT_HEART:
			return 'H';
		case SUIT_CLUB:
			return 'C';
		case SUIT_DIAMOND:
			return 'D';
		default:
			return '\0';
		}
	}

	std::ostream& operator << (std::ostream &stream, const State &state)
	{
		// Free cells
		for (int i = 0; i < 4; i++)
		{
			CARD card = state.CardInFreeCell(i);
			if (IsCard(card))
			{
				stream << FormatRank(RankOf(card));
				stream << FormatSuit(SuitOf(card));
			}
			else
			{
				stream << "--";
			}
			stream << ' ';
		}

		// Home cells
		for (int i = 0; i < 4; i++)
		{
			CARD card = state.TopCardInHomeCell(static_cast<Suit>(i));
			if (IsCard(card))
			{
				stream << FormatRank(RankOf(card));
				stream << FormatSuit(SuitOf(card));
			}
			else
			{
				stream << '-' << FormatSuit(static_cast<Suit>(i));
			}
			stream << ((i == 3) ? '\n' : ' ');
		}

		// Columns
		int columnDepth[8];
		CARD columnArea[8][52];

		for (int columnIndex = 0; columnIndex < 8; columnIndex++)
		{
			CARD card = state.TopCardOfColumn(columnIndex);
			int depth = 0;
			while (IsCard(card))
			{
				columnArea[columnIndex][depth++] = card;
				card = state.CardUnder(card);
			}
			columnDepth[columnIndex] = depth;
		}

		int maxDepth = *std::max_element(&columnDepth[0], &columnDepth[8]);

		for (int depth = 1; depth <= maxDepth; ++depth)
		{
			for (int columnIndex = 0; columnIndex < 8; columnIndex++)
			{
				int n = columnDepth[columnIndex];
				if (n < depth)
				{
					stream << "  ";
				}
				else
				{
					CARD card = columnArea[columnIndex][n - depth];
					stream << FormatRank(RankOf(card));
					stream << FormatSuit(SuitOf(card));
				}
				stream << ((columnIndex == 7) ? '\n' : ' ');
			}
		}

		stream << std::endl; // emit blank line
		return stream;
	}
}