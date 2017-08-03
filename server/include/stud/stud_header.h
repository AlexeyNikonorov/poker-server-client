#ifndef STUD_HEADER_H
#define STUD_HEADER_H

#include "card.h"
#include "deck.h"
#include "player.h"
#include "player_list.h"

enum { STUD_SUCCESS = 0,
	   STUD_FAILURE = -1,
	   STUD_PLAYER_LIMIT = 8,
	   STUD_STAGE_BRING_IN = 0,
	   STUD_STAGE_THIRD_STREET = 1,
	   STUD_STAGE_FOURTH_STREET = 2,
	   STUD_STAGE_FIFTH_STREET = 3,
       STUD_STAGE_SIXTH_STREET = 4,
       STUD_STAGE_SEVENTH_STREET = 5,
       STUD_ACTION_BRING_IN = 1,
       STUD_ACTION_FOLD = 2,
       STUD_ACTION_CALL = 4,
       STUD_ACTION_SB = 8,
       STUD_ACTION_BB = 16,
       STUD_ACTION_RAISE = 32,
	   BETTING_FAILURE = -1,
	   BETTING_SUCCESS = 0,
	   BETTING_RAISES_LIMIT = 3,
	   BETTING_OPTION_BRING_IN = 1,
	   BETTING_OPTION_FOLD = 2,
	   BETTING_OPTION_CHECK = 64,
	   BETTING_OPTION_CALL = 4,
	   BETTING_OPTION_SB = 8,
	   BETTING_OPTION_BB = 16,
	   BETTING_OPTION_RAISE = 32,
	   BETTING_STATE_BRING_IN = 1,
	   BETTING_STATE_BB_ALLOWED = 2,
	   BETTING_STATE_BET_MADE = 4, };

typedef struct stud_action {
	int kind;
	float value;
} stud_action_t;

typedef struct betting {
	player_list_t *players;
	float pot;
	float high_bet;
	int counter;
	int state;
	int stage;
	int raises;
	float sb;
	float bb;
	float bring_in;
	float ante;
} betting_t;

typedef struct stud {
	player_list_t *players;
	betting_t *betting;
    deck_t *deck;
    player_t *betting_opener;
	int stage;
	int curr_players;
	int max_players;
} stud_t;

static stud_t *stud_alloc(int max_players, float sb, float bb, float bring_in, float ante);
static void stud_free(stud_t *stud);
static int stud_add_player(stud_t *stud, player_t *player);
static int stud_arrange(stud_t *stud);
static void stud_next_stage(stud_t *stud);
static int stud_handle_action(stud_t *stud, player_t *player, stud_action_t *action);

static betting_t *betting_alloc(player_list_t *players, float sb, float bb, float bring_in, float ante);
static void betting_free(betting_t *betting);
static void betting_init(betting_t *betting);
static void betting_round_begin(betting_t *betting, int stage);
static int betting_round_is_finished(betting_t *betting);
static int betting_options(betting_t *betting, player_t *player);
static void betting_accept_ante(betting_t *betting, player_t *player);
static int betting_accept_action(betting_t *betting, player_t *player, stud_action_t *action);

static void stud_response_betting(stud_t *stud);

static player_t *hand_checker_pick_active_player_bring_in(player_list_t *players);
static player_t *hand_checker_pick_active_player(player_list_t *players, int *high_hand);
static int hand_checker_winners(player_list_t *players, player_t **winners);

#endif