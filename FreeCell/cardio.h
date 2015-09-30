#ifndef FCPLAY_CARDIO_H
#define FCPLAY_CARDIO_H

#include "card.h"
#include "state.h"

const char *card2str(CARD c);
const char *move2str(CARD_MOVE m);
const char *loc2str(CARD_LOCATION loc);

int  decode_cards(CARD cards[], int max_cards, const char *s);
int  ReadState(STATE *st, const char *filename);
void PrintState(const STATE *st);

#endif