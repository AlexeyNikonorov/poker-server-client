#ifndef HOLDEM_BETTING_H
#define HOLDEM_BETTING_H

#include "holdem_header.h"
#include "holdem_player.h"
#include "holdem_player_list.h"
#include <stdlib.h>

enum {
	BETTING_OPTION_FOLD = 1,
	BETTING_OPTION_CALL = 2,
	BETTING_OPTION_CHECK = 4,
	BETTING_OPTION_RAISE = 8,
	BETTING_OPTION_RERAISE = 16,
	BETTING_OPTION_ALLIN = 32,
	BETTING_STAGE_NONE = 0,
	BETTING_STAGE_PREFLOP = 1,
	BETTING_STAGE_THIRD_STREET = 2,
	BETTING_STAGE_FOURTH_STREET = 3,
	BETTING_STAGE_FIFTH_STREET = 4,
	BETTING_STAGE_SHOWDOWN = 5,
	BETTING_STAGE_ALLIN = 6,
};

/*
typedef struct {
	int kind;
	float value;
} holdem_action_t;

typedef struct {
	holdem_player_list_t *players;
	float sblind;
	float bblind;
	float high_bet;
	float pot;
	int counter;
	int nraises;
	int stage;
} holdem_betting_t;
*/
holdem_betting_t *holdem_betting_alloc(holdem_player_list_t *players, float sblind, float bblind) {
	holdem_betting_t *betting;

	betting = malloc(sizeof *betting);
	if (betting == NULL) {
		return NULL;
	}
	betting->players = players;
	betting->sblind = sblind;
	betting->bblind = bblind;
	betting->high_bet = 0;
	betting->pot = 0;
	betting->counter = 0;
	betting->nraises = 0;
	betting->stage = 0;
	return betting;
}

void holdem_betting_free(holdem_betting_t *betting) {
	free(betting);
}

int holdem_betting_round_begin(holdem_betting_t *betting) {
	betting->counter = holdem_player_list_count(betting->players);
	betting->nraises = 0;
	return 0;
}

int holdem_betting_round_is_finished(holdem_betting_t *betting) {
	return betting->counter == 0 ? 1 : 0;
}

int holdem_betting_options(holdem_betting_t *betting, holdem_player_t *player) {
	int options;

	options = 0;
	if (player->bet < betting->high_bet) {
		options |= BETTING_OPTION_FOLD;
		options |= BETTING_OPTION_CALL;
	} else if (player->bet == betting->high_bet) {
		options |= BETTING_OPTION_CHECK;
		options |= BETTING_OPTION_CALL;
	} else {
		return 0;
	}
	if (player->stack != 0.0) {
		if (betting->nraises == 0) {
			options |= BETTING_OPTION_RAISE;
		} else {
			options |= BETTING_OPTION_RERAISE;
			options |= BETTING_OPTION_RAISE;
		}
		options |= BETTING_OPTION_ALLIN;
	}
	return options;
}

int holdem_betting_accept_action(holdem_betting_t *betting,
								 holdem_player_t  *player,
								 holdem_action_t  *action) {
	float bet;
	int options;

	options = holdem_betting_options(betting, player);
	if (options == 0) {
		return -1;
	}
	switch (action->kind & options) {
		case BETTING_OPTION_FOLD:
			player->status = HOLDEM_PLAYER_STATUS_FOLD;
			betting->counter--;
			return 0;
		case BETTING_OPTION_CALL:
			bet = holdem_player_make_bet(player, betting->high_bet - player->bet);
			betting->pot += bet;
			player->status = HOLDEM_PLAYER_STATUS_COMMON;
			betting->counter--;
			return 0;
		case BETTING_OPTION_CHECK:
			player->status = HOLDEM_PLAYER_STATUS_COMMON;
			betting->counter--;
			return 0;
		case BETTING_OPTION_RAISE:
		case BETTING_OPTION_RERAISE:
			bet = holdem_player_make_bet(player, betting->high_bet + action->value - player->bet);
			betting->pot += bet;
			betting->high_bet += action->value;
			betting->nraises++;
			player->status = HOLDEM_PLAYER_STATUS_COMMON;
			betting->counter = holdem_player_list_count(betting->players) - 1;
			return 0;
		case BETTING_OPTION_ALLIN:
			bet = holdem_player_make_bet(player, player->stack);
			betting->pot += bet;
			betting->stage = BETTING_STAGE_ALLIN;
			betting->counter = 0;
			return 0;
		default:
			return -1;
	}
}

#endif