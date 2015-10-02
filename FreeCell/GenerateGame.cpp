///////////////////////////////////////////////////////////////////////////
// GenerateGame.cpp -- generate game from seed

/**
 * http://solitairelaboratory.com/mshuffle.txt
 * 
 * From: jimh@exmsft.com (Jim Horne)
 * 
 * I'm happy to share the card shuffle algorithm, but I warn you,
 * it does depend on the rand() and srand() function built into MS
 * compilers.  The good news is that I believe these work the same
 * for all our compilers.
 * 
 * I use cards.dll which has it's own mapping of numbers (0-51) to
 * cards.  The following will give you the idea.  Play around with
 * this and you'll be able to generate all the games.
 * 
 * Go ahead and post the code.  People might as well have fun with it.
 * Please keep me posted on anything interesting that comes of it.  
 * Thanks.
 */

/* Modified by Rongge */

#include <stdlib.h>
#include "Card.h"
#include "State.h"
#include "GenerateGame.h"

namespace FreeCell
{
	//static void generate_cards_raw(int gamenumber)
	//{
	//	int i, wLeft = 52;
	//	CARD deck[52];

	//	/* construct the deck in Jim's way */
	//	for (i = 0; i < 13; i++) {
	//		deck[4 * i] = MAKE_CARD(i + 1, CARD_CLUB);
	//		deck[4 * i + 1] = MAKE_CARD(i + 1, CARD_DIAMOND);
	//		deck[4 * i + 2] = MAKE_CARD(i + 1, CARD_HEART);
	//		deck[4 * i + 3] = MAKE_CARD(i + 1, CARD_SPADE);
	//	}

	//	/* shuffle cards */
	//	srand(gamenumber);
	//	for (i = 0; i < 52; i++) {
	//		int j = rand() % wLeft;
	//		gencards[i % 8][i / 8] = deck[j];
	//		deck[j] = deck[--wLeft];
	//	}
	//}

	static void GenerateGameImpl(int gameNumber, CARD gencards[8][7])
	{
		/* construct the deck in Jim's way */
		CARD deck[52];
		for (int rank = 1; rank <= 13; rank++)
		{
			deck[4 * (rank - 1) + 0] = MakeCard(static_cast<Rank>(rank), SUIT_CLUB);
			deck[4 * (rank - 1) + 1] = MakeCard(static_cast<Rank>(rank), SUIT_DIAMOND);
			deck[4 * (rank - 1) + 2] = MakeCard(static_cast<Rank>(rank), SUIT_HEART);
			deck[4 * (rank - 1) + 3] = MakeCard(static_cast<Rank>(rank), SUIT_SPADE);
		}

		/* shuffle cards */
		int wLeft = 52;
		srand(gameNumber);
		for (int i = 0; i < 52; i++) {
			int j = rand() % wLeft;
			gencards[i % 8][i / 8] = deck[j];
			deck[j] = deck[--wLeft];
		}
	}

	State GenerateGame(int gameNumber)
	{
		CARD gencards[8][7];
		GenerateGameImpl(gameNumber, gencards);

		State state;
		for (int i = 0; i < 8; i++) 
		{	
			for (int j = 0; j < ((i < 4) ? 7 : 6); j++) 
			{
				state.PlaceCardInColumn(gencards[i][j], i);
			}
		}
		return state;
	}
}