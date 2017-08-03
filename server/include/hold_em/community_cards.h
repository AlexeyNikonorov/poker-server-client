#ifndef COMMUNITY_CARDS_H
#define COMMUNITY_CARDS_H

#include "card.h"
#include "deck.h"

typedef struct community_cards {
	int calls_to_update;
	int calls;
	int len;
	card_t cards[5];
} community_cards_t;

static void community_cards_init(community_cards_t* comm_cards, int calls_to_update) {
	comm_cards->calls_to_update = calls_to_update;
	comm_cards->calls = 0;
	comm_cards->len = 0;
}

static void community_cards_reset(community_cards_t* comm_cards) {
    comm_cards->calls = 0;
    comm_cards->len = 0;
}

static community_cards_t* community_cards_alloc() {
	return malloc(sizeof(community_cards_t));
}

static int community_cards_update(community_cards_t* comm_cards, deck_t* deck) {
	if (comm_cards->len == 0) {
		deck_throw_card(deck, comm_cards->cards);
		deck_throw_card(deck, comm_cards->cards+1);
		deck_throw_card(deck, comm_cards->cards+2);
		return comm_cards->len = 3;
	} else if (comm_cards->len < 5) {
		deck_throw_card(deck, comm_cards->cards + comm_cards->len);
		return ++comm_cards->len;
	} else if (comm_cards->len == 5) {
	    return -1;
	}
	return 0;
}

#endif
