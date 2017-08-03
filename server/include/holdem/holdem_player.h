#ifndef HOLDEM_PLAYER_H
#define HOLDEM_PLAYER_H

#include "holdem_header.h"
#include "card.h"
#include "deck.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

enum {
	HOLDEM_PLAYER_STATUS_FOLD = 1,
	HOLDEM_PLAYER_STATUS_COMMON = 2,
	HOLDEM_PLAYER_STATUS_ACTIVE = 4,
};
/*
typedef struct {
	int conn;
	int id;
	char *name;
	int status;
	int position;
	float bet;
	float stack;
	card_t hand[2];
} holdem_player_t;
*/
holdem_player_t *holdem_player_alloc() {
	holdem_player_t *player;

	player = malloc(sizeof *player);
	if (player == NULL) {
		return NULL;
	}
	memset(player, 0, sizeof *player);
	return player;
}

void holdem_player_free(holdem_player_t *player) {
	free(player);
}

float holdem_player_make_bet(holdem_player_t *player, float bet) {
	float stack_cpy;

	if (bet < player->stack) {
		player->bet += bet;
		player->stack -= bet;
		return bet;
	} else {
		stack_cpy = player->stack;
		player->bet += stack_cpy;
		player->stack = 0;
		return stack_cpy;
	}
}

void holdem_player_pick_cards(holdem_player_t *player, deck_t *deck) {
	deck_throw_card(deck, player->hand);
	deck_throw_card(deck, player->hand+1);
}

#endif