#ifndef COMM_CARDS_H
#define COMM_CARDS_H

#include "holdem_header.h"
#include "card.h"
#include "deck.h"
#include <stdlib.h>
#include <string.h>
/*
typedef struct {
	card_t cards[5];
	int n;
} comm_cards_t;
*/
comm_cards_t *comm_cards_alloc() {
	comm_cards_t *comm_cards;

	comm_cards = malloc(sizeof *comm_cards);
	if (comm_cards == NULL) {
		return NULL;
	}
	memset(comm_cards, 0, sizeof *comm_cards);
	return comm_cards;
}

void comm_cards_free(comm_cards_t *comm_cards) {
	free(comm_cards);
}

void comm_cards_init(comm_cards_t *comm_cards) {
	memset(comm_cards, 0, sizeof *comm_cards);
}

int comm_cards_pick(comm_cards_t *comm_cards, deck_t *deck, int n) {
	int i, idx;

	if (comm_cards->n + n > 5) {
		return -1;
	}
	for (i = 0; i < n; i++) {
		idx = comm_cards->n + i;
		deck_throw_card(deck, &comm_cards->cards[idx]);
	}
	comm_cards->n += n;
	return 0;
}

int comm_cards_update(comm_cards_t *comm_cards, deck_t *deck) {
	if (comm_cards->n == 0) {
		deck_throw_card(deck, comm_cards->cards);
		deck_throw_card(deck, comm_cards->cards+1);
		deck_throw_card(deck, comm_cards->cards+2);
		comm_cards->n = 3;
		return comm_cards->n;
	} else if (comm_cards->n < 5) {
		deck_throw_card(deck, comm_cards->cards + comm_cards->n);
		comm_cards->n++;
		return comm_cards->n;
	} else if (comm_cards->n == 5) {
	    return -1;
	}
	return 0;
}

#endif