#ifndef DECK_H
#define DECK_H

#include "card.h"
#include <time.h>
#include <stdlib.h>

typedef struct deck {
	const card_t* top;
	const card_t* end;
	card_t cards[52];
} deck_t;

static void deck_init(deck_t* deck) {
	int suit, value;
	int i = 0;
	deck->top = deck->cards + 51;
	deck->end = deck->cards;
	for (suit = 0; suit < 4; suit++)
		for (value = 2; value < 15; value++)
			card_init(deck->cards + (i++), suit, value);
}

static deck_t* deck_alloc() {
	deck_t* deck = malloc(sizeof(deck_t));
	if (deck == NULL)
		return NULL;
	deck_init(deck);
	return deck;
}

static void deck_shuffle(deck_t* deck) {
	void swap(card_t* c1, card_t* c2) {
		card_t tmp = *c1;
		*c1 = *c2;
		*c2 = tmp;
	}
	const int n_swaps = 100;
	const int n_cards = 52;
	int i, j, k;
	
	deck->top = deck->cards + 51;
	deck->end = deck->cards;
	for (i = 0; i < n_swaps; i++) {
		j = rand() % n_cards;
		k = i % n_cards;
		if (j != k)
		    swap(deck->cards + j, deck->cards + k);
	}
}

static void deck_throw_card(deck_t* deck, card_t* card) {
	*card = *deck->top;
	if (deck->top == deck->end)
		deck_shuffle(deck);
	else
		deck->top--;
}

static void deck_print(const deck_t* deck) {
	int i;
	for (i = 0; i < 52; i++)
		card_print(deck->cards + i);
}

#endif
