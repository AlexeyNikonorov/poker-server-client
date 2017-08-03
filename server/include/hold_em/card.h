#ifndef CARD_H
#define CARD_H

#include <stdio.h>
#include <stdlib.h>

typedef struct card {
	int s;
	int v;
} card_t;

static int card_init(card_t* card, int suit, int value) {
	if (!card || suit > 3 || value < 2 || value > 14)
		return 0;
	card->s = suit;
	card->v = value;
	return 1;
}

static void card_to_string(const card_t* card, char s[]) {
	switch (card->s) {
		case 0:
			s[0] = 's';
			break;
		case 1:
			s[0] = 'c';
			break;
		case 2:
			s[0] = 'd';
			break;
		case 3:
			s[0] = 'h';
			break;
		default:
			return;
	}
	switch (card->v) {
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			s[1] = (char) (48 + card->v);
			s[2] = '\0';
			return;
		case 10:
			s[1] = '1';
			s[2] = '0';
			s[3] = '\0';
			return;
		case 11:
			s[1] = 'j';
			s[2] = '\0';
			return;
		case 12:
			s[1] = 'q';
			s[2] = '\0';
			return;
		case 13:
			s[1] = 'k';
			s[2] = '\0';
			return;
		case 14:
			s[1] = 'a';
			s[2] = '\0';
			return;
		default:
			return;
	}
}

static void card_print(const card_t* card) {
	printf("%d.%d\n", card->s, card->v);
}

#endif
