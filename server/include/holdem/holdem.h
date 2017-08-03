#ifndef HOLDEM_H
#define HOLDEM_H

#include "holdem_header.h"
#include "holdem_response.h"
#include "card.h"
#include "deck.h"
#include "comm_cards.h"
#include "hand_checker.h"
#include "holdem_player.h"
#include "holdem_player_list.h"
#include "holdem_betting.h"
#include <time.h>

void (*holdem_on_new_player)(holdem_t*, holdem_player_t*) = NULL;
void (*holdem_on_arrange)(holdem_t*) = NULL;
void (*holdem_on_betting)(holdem_t*) = NULL;
void (*holdem_on_stage)(holdem_t*) = NULL;
void (*holdem_on_showdown)(holdem_t*) = NULL;
void (*holdem_on_end)(holdem_t*, holdem_player_t**, int, float) = NULL;

holdem_t *holdem_alloc(int max_players, float sblind, float bblind) {
	holdem_t *holdem;

	holdem 				= 	malloc(sizeof *holdem);
	holdem->deck 		= 	deck_alloc();
	holdem->comm_cards 	= 	comm_cards_alloc();
	holdem->players 	= 	holdem_player_list_alloc(max_players);
	holdem->betting 	= 	holdem_betting_alloc(holdem->players, sblind, bblind);
	holdem->dealer 		= 	-1;
	return holdem;
}

void holdem_free(holdem_t *holdem) {
	free(holdem->deck);
	free(holdem->comm_cards);
	free(holdem->players);
	free(holdem->betting);
	free(holdem);
}

int holdem_add_player(holdem_t *holdem, holdem_player_t *player) {
	return holdem_player_list_push(holdem->players, player);
}

int holdem_cleanup(holdem_t *holdem) {
	return 0;
}

int holdem_arrange(holdem_t *holdem) {
	holdem_player_t 	*player;
	int 				n, i;

	if (holdem->players->nplayers < 2) {
		return -1;
	}
	holdem->betting->high_bet = holdem->betting->bblind;
	holdem->betting->pot = holdem->betting->sblind + holdem->betting->bblind;
	deck_shuffle(holdem->deck);
	comm_cards_init(holdem->comm_cards);
	if (holdem->dealer < 0) {
		holdem->dealer = rand() % holdem->players->nplayers;
	} else {
		holdem->dealer = (holdem->dealer + 1) % holdem->players->nplayers;
	}
	holdem_player_list_set_pos(holdem->players, holdem->dealer);
	n = holdem->players->nplayers < 4 ? 4 : holdem->players->nplayers;
	for (i = 0; i < n; i++) {
		player = holdem_player_list_get(holdem->players, i);
		if (i < holdem->players->nplayers) {
			player->bet = 0.0;
			holdem_player_pick_cards(player, holdem->deck);
		}
		if (i == 1) {
			holdem_player_make_bet(player, holdem->betting->sblind);
		} else if (i == 2) {
			holdem_player_make_bet(player, holdem->betting->bblind);
		}
		if (i == 3) {
			player->status = HOLDEM_PLAYER_STATUS_ACTIVE;
		} else {
			player->status = HOLDEM_PLAYER_STATUS_COMMON;
		}
	}
	holdem_player_list_inc_pos(holdem->players, 3);
	holdem->betting->stage = BETTING_STAGE_PREFLOP;
	holdem_betting_round_begin(holdem->betting);
	if (holdem_on_arrange != NULL) {
		holdem_on_arrange(holdem);
	}
	return 0;
}

int holdem_end_all_fold(holdem_t *holdem) {
	holdem_player_t 	*winner;
	float 				gain;

	winner = holdem_player_list_get(holdem->players, 0);
	gain = holdem->betting->pot;
//RESPONSE END
	if (holdem_on_end != NULL) {
		sleep(2);
		holdem_on_end(holdem, &winner, 1, gain);
	}
	winner->stack += gain;
	holdem_cleanup(holdem);
	return 0;
}

int holdem_end_showdown(holdem_t *holdem) {
	holdem_player_t		*player;
	holdem_player_t 	**winners;
	card_t 				hand[7];
	uint32_t 			cur, max;
	int 				nplayers, nwinners, i;
	float 				gain;

//RESPONSE SHOWDOWN
	if (holdem_on_showdown != NULL) {
		sleep(2);
		holdem_on_showdown(holdem);
	}
	hand[2] = holdem->comm_cards->cards[0];
	hand[3] = holdem->comm_cards->cards[1];
	hand[4] = holdem->comm_cards->cards[2];
	hand[5] = holdem->comm_cards->cards[3];
	hand[6] = holdem->comm_cards->cards[4];
	nplayers = holdem_player_list_count(holdem->players);
	winners = malloc(nplayers*sizeof *winners);
	nwinners = 0;
	max = 0;
	for (i = 0; i < nplayers; i++) {
		player = holdem_player_list_get(holdem->players, i);
		hand[0] = player->hand[0];
		hand[1] = player->hand[1];
		cur = hand_checker(hand, 7);
		if (cur > max) {
			max = cur;
			nwinners = 1;
			winners[0] = player;
		} else if (cur == max) {
			nwinners++;
			winners[nwinners] = player;
		}
	}
	gain = holdem->betting->pot / nwinners;
//RESPONSE WINNERS
	if (holdem_on_end != NULL) {
		sleep(2);
		holdem_on_end(holdem, winners, nwinners, gain);
	}
	for (i = 0; i < nwinners; i++) {
		winners[i]->stack += gain;
	}
	free(winners);
	holdem_cleanup(holdem);
	return 0;
}

int holdem_next_stage(holdem_t *holdem) {
	switch (++holdem->betting->stage) {
		case BETTING_STAGE_THIRD_STREET:
			comm_cards_pick(holdem->comm_cards, holdem->deck, 3);
			holdem_betting_round_begin(holdem->betting);
			if (holdem_on_stage != NULL) {
				holdem_on_stage(holdem);
			}
			return 0;
		case BETTING_STAGE_FOURTH_STREET:
		case BETTING_STAGE_FIFTH_STREET:
			comm_cards_pick(holdem->comm_cards, holdem->deck, 1);
			holdem_betting_round_begin(holdem->betting);
			if (holdem_on_stage != NULL) {
				holdem_on_stage(holdem);
			}
			return 0;
		case BETTING_STAGE_SHOWDOWN:
			return holdem_end_showdown(holdem);
		default:
			return -1;
	}
}

int holdem_handle_action(holdem_t *holdem, holdem_player_t *player, holdem_action_t *action) {
	if (player->status != HOLDEM_PLAYER_STATUS_ACTIVE) {
		return -1;
	}
	if (holdem_on_betting != NULL) {
		holdem_on_betting(holdem);
	}
	holdem_betting_accept_action(holdem->betting, player, action);
	holdem_player_list_next(holdem->players);
	if (holdem_player_list_count(holdem->players) == 1) {
		return holdem_end_all_fold(holdem);
	}
	if (holdem_betting_round_is_finished(holdem->betting)) {
		holdem_next_stage(holdem);
	}
	return 0;
}

#endif