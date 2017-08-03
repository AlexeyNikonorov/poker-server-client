#ifndef HOLD_EM
#define HOLD_EM

#include "card.h"
#include "deck.h"
#include "community_cards.h"
#include "player.h"
#include "player_list.h"
#include "hand_checker.h"
#include <stdlib.h>

enum { HOLD_EM_STAGE_COMMON = 0,
       HOLD_EM_STAGE_FLOP = 3,
       HOLD_EM_STAGE_TURN = 4,
       HOLD_EM_STAGE_RIVER = 5,
       HOLD_EM_STAGE_END_FOLD = -1,
       HOLD_EM_STAGE_END_SHOWDOWN = -2, };

typedef struct hold_em {
	int sblind;
	int bblind;
	int max_players;
	int curr_players;
	int dealer;
	int max_odd;
	int pot;
	deck_t* deck;
	community_cards_t* comm_cards;
	player_list_t* players;
} hold_em_t;

static void hold_em_free(hold_em_t* hold_em) {
	free(hold_em->players);
	free(hold_em->comm_cards);
	free(hold_em->deck);
	free(hold_em);
}

static hold_em_t* hold_em_alloc(int sblind, int bblind, int max_players) {
	hold_em_t* hold_em;
	
	hold_em = malloc(sizeof(hold_em_t));
	if (hold_em == NULL)
		return NULL;
	hold_em->sblind = sblind;
	hold_em->bblind = bblind;
	hold_em->max_players = max_players;
	hold_em->curr_players = 0;
	hold_em->dealer = -1;
	hold_em->deck = deck_alloc();
	hold_em->comm_cards = community_cards_alloc();
	hold_em->players = player_list_alloc(max_players);
	if (!hold_em->deck || !hold_em->comm_cards || !hold_em->players) {
		hold_em_free(hold_em);
		return NULL;
	}
	return hold_em;
}

static int hold_em_add_player(hold_em_t* hold_em, player_t* player) {
	if (hold_em->curr_players == hold_em->max_players)
		return 0;
	player->client_status = PLAYER_STATUS_COMMON;
	player_list_push(hold_em->players, player);
	hold_em->curr_players++;
	return 1;
}

static void hold_em_cleanup(hold_em_t* hold_em) {
    player_t* player;
    int i;
    
    for (i = 0; i < hold_em->curr_players; i++) {
        player = player_list_get(hold_em->players, i);
        if (player->client_status == PLAYER_STATUS_OFFLINE) {
	        player_list_remove(hold_em->players, player);
	        hold_em->curr_players--;
    	}
    }
}

static int hold_em_arrange(hold_em_t* hold_em) {
    player_t* player;
    int n, i;

    if (hold_em->curr_players < 2) {
	    return 0;
    }

    hold_em->max_odd = hold_em->bblind;
    hold_em->pot = hold_em->sblind + hold_em->bblind;
    deck_shuffle(hold_em->deck);
    community_cards_init(hold_em->comm_cards, hold_em->curr_players);

    if (hold_em->dealer < 0) {
	    hold_em->dealer = rand() % hold_em->curr_players;
    } else {
	    hold_em->dealer = (hold_em->dealer + 1) % hold_em->curr_players;
    }

    player_list_set_pos(hold_em->players, hold_em->dealer);
    n = (hold_em->curr_players < 4) ? 4 : hold_em->curr_players;

    for (i = 0; i < n; i++) {
	    player = player_list_get(hold_em->players, i);
	    player->position = i;
	
	    if (i < hold_em->curr_players) {
		    player->odd = 0;
		    player_pick_cards(player, hold_em->deck);
	    }
	
	    if (i == 1) {
		    player_make_bet(player, hold_em->sblind);
	    } else if (i == 2) {
		    player_make_bet(player, hold_em->bblind);
	    }
	
	    player->status = (i == 3) ? PLAYER_STATUS_ACTIVE : PLAYER_STATUS_COMMON;
    }

    player_list_inc_pos(hold_em->players, 3);
    return 1;
}

static int hold_em_handle_action(hold_em_t* hold_em, player_t* player, action_request_t* data) {
	if (player->status != PLAYER_STATUS_ACTIVE) {
		return 0;
	}
	
	switch (data->kind) {
		case ACTION_DATA_FOLD:
			if (player->odd >= hold_em->max_odd)
				return 0;
			player->status = PLAYER_STATUS_FOLD;
			return 1;
		case ACTION_DATA_CALL:
			hold_em->pot += player_make_bet(player, hold_em->max_odd - player->odd);
			player->status = PLAYER_STATUS_COMMON;
			return 1;
		case ACTION_DATA_RAISE:
			hold_em->pot += player_make_bet(player, hold_em->max_odd + data->val - player->odd);
			hold_em->max_odd += data->val;
			player->status = PLAYER_STATUS_COMMON;
			return 1;
		default:
		    puts("error in hold_em_handle_action");
			return 0;
	}
} 

static int hold_em_stage(hold_em_t* hold_em, action_request_t* data) {
	int stage;

    if (player_list_count(hold_em->players) == 1) {
        return HOLD_EM_STAGE_END_FOLD;
    }
    
    switch (data->kind) {
        case ACTION_DATA_FOLD:
        case ACTION_DATA_CALL:
            hold_em->comm_cards->calls++;
            if (hold_em->comm_cards->calls_to_update != hold_em->comm_cards->calls) {
            	player_list_pop(hold_em->players)->status = PLAYER_STATUS_ACTIVE;
                return HOLD_EM_STAGE_COMMON;
            }

            stage = community_cards_update(hold_em->comm_cards, hold_em->deck);
            if (stage < 0) {
        		return HOLD_EM_STAGE_END_SHOWDOWN;
            }
            hold_em->comm_cards->calls = 0;
            hold_em->comm_cards->calls_to_update = player_list_count(hold_em->players);
            player_list_pop(hold_em->players)->status = PLAYER_STATUS_ACTIVE;
            return stage;
        case ACTION_DATA_RAISE:
		    hold_em->comm_cards->calls = 1;
		    hold_em->comm_cards->calls_to_update = player_list_count(hold_em->players);
		    player_list_pop(hold_em->players)->status = PLAYER_STATUS_ACTIVE;
		    return HOLD_EM_STAGE_COMMON;
	    default:
	        puts("error in hold_em_stage (1)");
	        return 0;
    }
}

static void hold_em_end_fold(hold_em_t* hold_em, const char** winners, int* gain) {
    player_t* player = player_list_pop(hold_em->players);
    player->stack += hold_em->pot;
    *gain = hold_em->pot;
    winners[0] = player->name;
    winners[1] = NULL;
    community_cards_reset(hold_em->comm_cards);
    hold_em_cleanup(hold_em);
}

static void hold_em_end_showdown(hold_em_t* hold_em, const char** winners, int* gain) {
    const int revealed = player_list_count(hold_em->players);
    player_hand *result;
    player_t* player;
    int draw, i;
    
    result = alloca(revealed * sizeof(player_hand));
    
    for (i = 0; i < revealed; i++) {
        player = player_list_pop(hold_em->players);
        result[i].name = player->name;
        hand_checker(hold_em->comm_cards->cards, player->hand, result[i].res);
        player_hand_print(result + i);
    }
    
    draw = cmp_results(result, winners, revealed);
    *gain = hold_em->pot / draw;
    
    for (i = 0; i < draw; i++) {
        player = player_list_get_by_name(hold_em->players, winners[i]);
        player->stack += *gain;
    }

    community_cards_reset(hold_em->comm_cards);
    hold_em_cleanup(hold_em);
}

#endif
