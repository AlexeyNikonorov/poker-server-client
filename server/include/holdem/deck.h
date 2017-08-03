#ifndef DECK_H
#define DECK_H

#include "holdem_header.h"
#include "card.h"
#include <time.h>
#include <stdlib.h>
/*
typedef struct deck {
	const card_t *top;
	const card_t *end;
	card_t cards[52];
} deck_t;
*/
static void deck_init(deck_t *deck) {
	int suit, value;
	int i = 0;

	deck->top = deck->cards + 51;
	deck->end = deck->cards;
	for (suit = 0; suit < 4; suit++)
		for (value = 2; value < 15; value++)
			card_init(deck->cards + (i++), suit, value);
}

static deck_t *deck_alloc() {
	deck_t *deck;

	deck = malloc(sizeof *deck);
	if (deck == NULL)
		return NULL;
	deck_init(deck);
	return deck;
}

static void deck_free(deck_t *deck) {
	free(deck);
}

static void deck_shuffle(deck_t *deck) {
	int nswaps = 100;
	int ncards = 52;
	int i, j, k;
	card_t copy;
	
	deck->top = deck->cards + 51;
	deck->end = deck->cards;
	for (i = 0; i < nswaps; i++) {
		j = rand() % ncards;
		k = i % ncards;
		if (j != k) {
			copy = deck->cards[j];
			deck->cards[j] = deck->cards[k];
			deck->cards[k] = copy;
		}
	}
}

static void deck_throw_card(deck_t *deck, card_t *card) {
	*card = *deck->top;
	if (deck->top == deck->end) {
		deck_shuffle(deck);
	} else {
		deck->top--;
	}
}

static void deck_print(deck_t *deck) {
	int i;
	for (i = 0; i < 52; i++)
		card_print(deck->cards + i);
}

#endif
