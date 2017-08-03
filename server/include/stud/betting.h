#ifndef BETTING_H
#define BETTING_H

#include "player.h"
#include "player_list.h"
#include "stud_header.h"
#include <stdio.h>

static void print_bin(int u) {
    const int size = 13;
    char b[size];
    int i;

    memset(b, 0, size);
    for (i = 0; i < size; i++)
        b[i] = (u & (1<<i)) ? '1' : '0';
    b[size] = '\0';
    printf("%s\n", b);
}

static betting_t *betting_alloc(player_list_t *players, float sb, float bb, float bring_in, float ante) {
	betting_t *betting;

	betting = malloc(sizeof(betting_t));
	if (betting == NULL) {
		return NULL;
	}

	betting->players = players;
	betting->sb = sb;
	betting->bb = bb;
	betting->bring_in = bring_in;
	betting->ante = ante;
	return betting;
}

static void betting_free(betting_t *betting) {
	free(betting);
}

static void betting_init(betting_t *betting) {
	betting->pot = 0.0;
	betting->high_bet = 0.0;
	betting->state = 0;
}

static void betting_round_begin(betting_t *betting, int stage) {
	betting->stage = stage;
	betting->counter = player_list_count(betting->players);
	betting->raises = 0;
	if (stage == STUD_STAGE_THIRD_STREET) {
		betting->state |= BETTING_STATE_BRING_IN;
	} else {
		betting->state &= ~BETTING_STATE_BET_MADE;
	}
}

static int betting_round_is_finished(betting_t *betting) {
	return (betting->counter == 0) ? 1 : 0;
}

static int betting_options(betting_t *betting, player_t *player) {
	int options = 0;

	if (betting->state & BETTING_STATE_BRING_IN) {
		options |= BETTING_OPTION_BRING_IN;
		options |= BETTING_OPTION_SB;
		return options;
	}

	if (player->bet == betting->high_bet) {
		options |= BETTING_OPTION_CHECK;
		options |= BETTING_OPTION_CALL;
	} else if (player->bet < betting->high_bet) {
		options |= BETTING_OPTION_FOLD;
		options |= BETTING_OPTION_CALL;
	}
	if (!(betting->state & BETTING_STATE_BET_MADE)) {
		options |= BETTING_OPTION_SB;
		if  (betting->state & BETTING_STATE_BB_ALLOWED) {
			options |= BETTING_OPTION_BB;
		}
	}
	if ((betting->state & BETTING_STATE_BET_MADE) && (betting->raises < BETTING_RAISES_LIMIT)) {
		options |= BETTING_OPTION_RAISE;
	}
	return options;
}

static void betting_accept_ante(betting_t *betting, player_t *player) {
	player->stack -= betting->ante;
	betting->pot += betting->ante;
}

static int betting_accept_action(betting_t *betting, player_t *player, stud_action_t *action) {
	float bet;
	int options;

	options = betting_options(betting, player);

	switch (action->kind & options) {
		case STUD_ACTION_BRING_IN:
			bet = player_make_bet(player, betting->bring_in);
			betting->high_bet = bet;
			betting->pot += bet;
			betting->state ^= BETTING_STATE_BRING_IN;
			player->status = PLAYER_STATUS_COMMON;
			betting->counter--;
			break;
		case STUD_ACTION_FOLD:
			player->status = PLAYER_STATUS_FOLD;
			betting->counter--;
			break;
		case STUD_ACTION_CALL:
			bet = player_make_bet(player, betting->high_bet - player->bet);
			betting->pot += bet;
			player->status = PLAYER_STATUS_COMMON;
			betting->counter--;
			break;
		case STUD_ACTION_SB:
			if (betting->stage == STUD_STAGE_THIRD_STREET) {
				bet = player_make_bet(player, betting->sb - player->bet);
				betting->pot += bet;
				betting->high_bet = betting->sb;
				betting->state &= ~BETTING_STATE_BRING_IN;
			} else {
				bet = player_make_bet(player, betting->high_bet + betting->sb - player->bet);
				betting->pot += bet;
				betting->high_bet += betting->sb;
			}
			betting->state |= BETTING_STATE_BET_MADE;
			player->status = PLAYER_STATUS_COMMON;
			betting->counter = player_list_count(betting->players) - 1;
			break;
		case STUD_ACTION_BB:
			bet = player_make_bet(player, betting->high_bet + betting->bb - player->bet);
			betting->pot += bet;
			betting->high_bet += betting->bb;
			betting->state |= BETTING_STATE_BET_MADE;
			player->status = PLAYER_STATUS_COMMON;
			betting->counter = player_list_count(betting->players) - 1;
			break;
		case STUD_ACTION_RAISE:
			bet = player_make_bet(player, betting->high_bet + action->value - player->bet);
			betting->pot += bet;
			betting->high_bet += action->value;
			betting->raises++;
			player->status = PLAYER_STATUS_COMMON;
			betting->counter = player_list_count(betting->players) - 1;
			break;
		default:
			printf("invalid action\n");
			return BETTING_FAILURE;
	}

	return BETTING_SUCCESS;
}

#endif