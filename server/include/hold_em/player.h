#ifndef PLAYER_H
#define PLAYER_H

#include "card.h"
#include "deck.h"
#include <unistd.h>
#include <stdlib.h>

enum { PLAYER_STATUS_OFFLINE = 0,
       PLAYER_STATUS_NEW = 1,
       PLAYER_STATUS_FOLD = 2,
	   PLAYER_STATUS_COMMON = 3,
	   PLAYER_STATUS_ACTIVE = 4 };

typedef struct player {
    int conn;
    int id;
    const char* name;
    int client_status;
    int status;
    int position;
    int odd;
    int stack;
    card_t hand[2];
} player_t;

static player_t* player_alloc(int odd, int stack) {
	player_t* player = malloc(sizeof(player_t));
	if (player == NULL)
		return NULL;
	player->name = NULL;
	player->status = PLAYER_STATUS_COMMON;
	player->odd = odd;
	player->stack = stack;
	return player;
}

static void player_free(player_t* player) {
    close(player->conn);
	free(player);
}

static int player_make_bet(player_t* player, int bet) {
	if (bet < player->stack) {
		player->odd += bet;
		player->stack -= bet;
		return bet;
	} else {
		int tmp_stack = player->stack;
		player->odd += tmp_stack;
		player->stack = 0;
		return tmp_stack;
	}
}

static void player_pick_cards(player_t* player, deck_t* deck) {
	deck_throw_card(deck, player->hand);
	deck_throw_card(deck, player->hand+1);
}

static void player_print(player_t* player) {
    printf("%s: %d/%d [%d.%d %d.%d]\n", player->name,
                                        player->odd, player->stack,
                                        player->hand[0].s, player->hand[0].v,
                                        player->hand[1].s, player->hand[1].v);
}

#endif
