#ifndef CARD_H
#define CARD_H

#include "holdem_header.h"
#include <stdio.h>

enum { CARD_SUITE_SPADES = 3,
	   CARD_SUITE_HEARTS = 2,
	   CARD_SUITE_DIAMONDS = 1,
	   CARD_SUITE_CLUBS = 0, };
/*
typedef struct card {
	int s;
	int v;
} card_t;
*/
static int card_init(card_t *card, int suit, int value) {
	if (!card || suit > 3 || value < 2 || value > 14)
		return 0;
	card->s = suit;
	card->v = value;
	return 1;
}

static void card_to_string(card_t *card, char *s) {
	switch (card->s) {
		case CARD_SUITE_SPADES: s[0] = 's'; break;
		case CARD_SUITE_HEARTS: s[0] = 'h'; break;
		case CARD_SUITE_DIAMONDS: s[0] = 'd'; break;
		case CARD_SUITE_CLUBS: s[0] = 'c'; break;
		default: s[0] = 'f'; s[1] = 'd'; s[2] = '\0'; return;
	}
	switch (card->v) {
		case 2: case 3: case 4: case 5:
		case 6: case 7: case 8: case 9: s[1] = (char)(48+card->v); s[2] = '\0'; return;
		case 10: s[1] = '1'; s[2] = '0'; s[3] = '\0'; return;
		case 11: s[1] = 'j'; s[2] = '\0'; return;
		case 12: s[1] = 'q'; s[2] = '\0'; return;
		case 13: s[1] = 'k'; s[2] = '\0'; return;
		case 14: s[1] = 'a'; s[2] = '\0'; return;
		default: s[0] = 'f'; s[1] = 'd'; s[2] = '\0'; return;
	}
}

static void card_print(card_t *card) {
	printf("%d.%d\n", card->s, card->v);
}

#endif