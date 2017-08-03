#ifndef STUD_H
#define STUD_H

#include "stud_header.h"

#include "card.h"
#include "deck.h"
#include "player.h"
#include "player_list.h"

#include "betting.h"
#include "hand_checker.h"
#include "response.h"

static stud_t *stud_alloc(int max_players, float sb, float bb, float bring_in, float ante) {
	stud_t *stud;

	if (max_players > STUD_PLAYER_LIMIT) {
		return NULL;
	}

	stud = malloc(sizeof(stud_t));
	if (stud == NULL) {
		return NULL;
	}

	stud->players = player_list_alloc(max_players);
	if (stud->players == NULL) {
		free(stud);
		return NULL;
	}

    stud->betting = betting_alloc(stud->players, sb, bb, bring_in, ante);
    if (stud->betting == NULL) {
        player_list_free(stud->players);
        free(stud);
        return NULL;
    }

	stud->deck = deck_alloc();
	if (stud->deck == NULL) {
		player_list_free(stud->players);
        betting_free(stud->betting);
		free(stud);
		return NULL;
	}

	stud->curr_players = 0;
	stud->max_players = max_players;
	return stud;
}
 
static void stud_free(stud_t *stud) {
	deck_free(stud->deck);
    betting_free(stud->betting);
	player_list_free(stud->players);
	free(stud);
}

static int stud_add_player(stud_t *stud, player_t *player) {
	if (player_list_push(stud->players, player) == PLAYER_LIST_FAIL) {
		return STUD_FAILURE;
	}
	player->status = PLAYER_STATUS_NEW;
	return ++stud->curr_players;
}

static void stud_cleanup(stud_t *stud) {
    player_list_foreach(stud->players,
        if (__player->client_status == PLAYER_STATUS_OFFLINE) {
            player_list_remove(stud->players, __player);
            stud->curr_players--;
        }
    );
}

static int stud_arrange(stud_t *stud) {
	if (stud->curr_players < 2) {
		return STUD_FAILURE;
	}

	stud->stage = STUD_STAGE_THIRD_STREET;
    betting_init(stud->betting);
    deck_shuffle(stud->deck);

	player_list_foreach(stud->players,
        player_init(__player);
        betting_accept_ante(stud->betting, __player);
        player_pick_downcard(__player, stud->deck);
        player_pick_downcard(__player, stud->deck);
        player_pick_upcard(__player, stud->deck);
	);

	stud->betting_opener = hand_checker_pick_active_player_bring_in(stud->players);
    betting_round_begin(stud->betting, stud->stage);
#ifndef NO_RESPONSE
    stud_response_betting(stud);
#endif
	return STUD_SUCCESS;
}

static void stud_end_game_all_fold(stud_t *stud) {
    player_t *winner;

    winner = player_list_get(stud->players, 0);
    sleep(2); stud_response_end(stud, &winner, 1, stud->betting->pot);

    winner->stack += stud->betting->pot;
    player_list_foreach(stud->players, __player->bet = 0);
    stud->betting->high_bet = 0;
    stud->betting->pot = 0;

    stud_cleanup(stud);

    sleep(2); stud_arrange(stud);
}

static void stud_end_game_showdown(stud_t *stud) {
    player_t **winners;
    int active_players, ties, i;
    float gain;

    sleep(2); stud_response_showdown(stud);

    active_players = player_list_count(stud->players);
    winners = alloca(active_players * sizeof(player_t*));
    ties = hand_checker_winners(stud->players, winners) + 1;
    gain = stud->betting->pot / ties;

    for (i = 0; i < ties; i++) {
        winners[i]->stack += gain;
    }

    sleep(2); stud_response_end(stud, winners, ties, gain);

    player_list_foreach(stud->players, __player->bet = 0);
    stud->betting->high_bet = 0;
    stud->betting->pot = 0;

    stud_cleanup(stud);

    sleep(2); stud_arrange(stud);
}

static void stud_next_stage(stud_t *stud) {
    int high_hand;

    switch (++stud->stage) {
        case STUD_STAGE_FOURTH_STREET:
        case STUD_STAGE_FIFTH_STREET:
        case STUD_STAGE_SIXTH_STREET:
            player_list_for_active(stud->players,
                player_pick_upcard(__player, stud->deck);
            );
            stud->betting_opener = hand_checker_pick_active_player(stud->players, &high_hand);
            if (high_hand >= PAIR) {
                stud->betting->state |= BETTING_STATE_BB_ALLOWED;
            }
            betting_round_begin(stud->betting, stud->stage);
            return;
            
        case STUD_STAGE_SEVENTH_STREET:
            player_list_for_active(stud->players,
                player_pick_downcard(__player, stud->deck);
            );
            stud->betting_opener->status = PLAYER_STATUS_ACTIVE;
            player_list_rewind(stud->players);
            betting_round_begin(stud->betting, stud->stage);
            return;

        default:
            puts("error in stud_next_stage (3)");
    }
}

static int stud_handle_action(stud_t *stud, player_t *player, stud_action_t *action) {
	if (player->status != PLAYER_STATUS_ACTIVE) {
		return STUD_FAILURE;
	}

	switch (stud->stage) {
		case STUD_STAGE_THIRD_STREET:
        case STUD_STAGE_FOURTH_STREET:
        case STUD_STAGE_FIFTH_STREET:
        case STUD_STAGE_SIXTH_STREET:
            if (betting_accept_action(stud->betting, player, action) == BETTING_FAILURE) {
                return STUD_FAILURE;
            }
            if (player_list_count(stud->players) == 1) {
                stud_end_game_all_fold(stud);
                return STUD_SUCCESS;
            }
            if (betting_round_is_finished(stud->betting)) {
                stud_next_stage(stud);
            } else {
                player_list_next(stud->players);
            }
#ifndef NO_RESPONSE
    stud_response_betting(stud);
#endif
            return STUD_SUCCESS;

        case STUD_STAGE_SEVENTH_STREET:
            if (betting_accept_action(stud->betting, player, action) == BETTING_FAILURE) {
                return STUD_FAILURE;
            }
            if (player_list_count(stud->players) == 1) {
                stud_end_game_all_fold(stud);
                return STUD_SUCCESS;
            }
            if (betting_round_is_finished(stud->betting)) {
                stud_end_game_showdown(stud);
                return STUD_SUCCESS;
            } else {
                player_list_next(stud->players);
            }
#ifndef NO_RESPONSE
    stud_response_betting(stud);
#endif
            return STUD_SUCCESS;

        default:
            puts("stud_handle_action: invalid game stage");
            return STUD_FAILURE;
	}
}

#endif