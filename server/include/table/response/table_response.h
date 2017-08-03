#ifndef TABLE_RESPONSE_H
#define TABLE_RESPONSE_H

#include "../table.h"
#include "table_parser.h"

#include <jansson.h>
#include <unistd.h>
#include <string.h>

void table_response_hold_em_new_player(hold_em_t *hold_em, player_t *new_player) {
	json_t *new_player_data, *players, *comm_cards, *root;
	player_t *player;
	char *response, card_string[4];
	size_t len;
	int i;
	
	new_player_data = json_pack("{s:i,s:s,s:i,s:i}",
							    "position", 0,
							    "name", new_player->name,
							 	"bet", new_player->odd,
							 	"stack", new_player->stack);
 	
 	root = json_pack("{s:o}", "new_player", new_player_data);
	response = json_dumps(root, JSON_COMPACT);
	len = strlen(response);
	for (i = 0; i < hold_em->curr_players; i++) {
		player = player_list_get(hold_em->players, i);
		if (strcmp(player->name, new_player->name) == 0)
		    continue;
		if (write(player->conn, response, len) != len)
			puts("error in table_response_hold_em_new_player (1)");
	}
	
	players = json_array();
	for (i = 0; i < hold_em->curr_players; i++) {
		player = player_list_get(hold_em->players, i);
		json_array_append_new(players, json_pack("{s:i,s:s,s:i,s:i}",
												 "position", player->position,
												 "name", player->name,
												 "bet", player->odd,
												 "stack", player->stack));
	}
	
	if (hold_em->comm_cards->len) {
	    comm_cards = json_array();
	    for (i = 0; i < hold_em->comm_cards->len; i++) {
	        card_to_string(hold_em->comm_cards->cards+i, card_string);
	        json_array_append_new(comm_cards, json_string(card_string));
	    }
	    
	    root = json_pack("{s:{s:o,s:o}}", "updates",
	                     "players", players,
	                     "flop", comm_cards);
	} else {
	    root = json_pack("{s:{s:o}}", "updates",
	                     "players", players);
	}
	
	response = json_dumps(root, JSON_COMPACT);
	len = strlen(response);
	if (write(new_player->conn, response, len) != len)
	    puts("error in table_response_hold_em_new_player (2)");
}

void table_response_hold_em_reconnect(hold_em_t *hold_em, player_t *player) {
    json_t *players, *active, *cards, *comm_cards, *root;
    player_t *player_i;
    char *response, card_string[4];
    size_t len;
    int i;
    
    players = json_array();
    for (i = 0; i < hold_em->curr_players; i++) {
        player_i = player_list_get(hold_em->players, i);
		json_array_append_new(players, json_pack("{s:i,s:s,s:i,s:i}",
												 "position", player_i->position,
												 "name", player_i->name,
												 "bet", player_i->odd,
												 "stack", player_i->stack));
    }
    
	active = json_pack("{s:i,s:s}",
					   "position", player_list_get(hold_em->players, 0)->position,
					   "name", player_list_get(hold_em->players, 0)->name);
   
    cards = json_array();
    card_to_string(player->hand, card_string);
    json_array_append_new(cards, json_string(card_string));
    card_to_string(player->hand+1, card_string);
    json_array_append_new(cards, json_string(card_string));
    

    if (hold_em->comm_cards->len) {
        comm_cards = json_array();
        for (i = 0; i < hold_em->comm_cards->len; i++) {
            card_to_string(hold_em->comm_cards->cards+i, card_string);
            json_array_append_new(comm_cards, json_string(card_string));
        }
        root = json_pack("{s:{s:o,s:o,s:o,s:o}}",
                         "reconnect",
                         "players", players,
                         "active", active,
                         "cards", cards,
                         "flop", comm_cards);
    } else {
        root = json_pack("{s:{s:o,s:o,s:o}}",
                         "reconnect",
                         "players", players,
                         "active", active,
                         "cards", cards);
    }
    
    response = json_dumps(root, JSON_COMPACT);
    len = strlen(response);
    if (write(player->conn, response, len) != len)
        puts("error in table_response_hold_em_data");
}

void table_response_hold_em_init(hold_em_t *hold_em) {
	json_t *players, *active, *cards, *root;
	player_t *player;
	char *response, card_string[4];
	size_t len;
	int i;
	
	players = json_array();
	for (i = 0; i < hold_em->curr_players; i++) {
		player = player_list_get(hold_em->players, i);
		json_array_append_new(players, json_pack("{s:i,s:s,s:i,s:i}",
												 "position", player->position,
												 "name", player->name,
												 "bet", player->odd,
												 "stack", player->stack));
	}
	
	active = json_pack("{s:i,s:s}",
					   "position", player_list_get(hold_em->players, 0)->position,
					   "name", player_list_get(hold_em->players, 0)->name);
					   
	for (i = 0; i < hold_em->curr_players; i++) {
		player = player_list_get(hold_em->players, i);
		cards = json_array();
		card_to_string(player->hand, card_string);
		json_array_append_new(cards, json_string(card_string));
		card_to_string(player->hand+1, card_string);
		json_array_append_new(cards, json_string(card_string));
		root = json_pack("{s:{s:o,s:o,s:o}}",
		 				 "init",
						 "players", players,
						 "active", active,
						 "cards", cards);
		response = json_dumps(root, JSON_COMPACT);
		len = strlen(response);
		if (write(player->conn, response, len) != len)
			puts("error in table_response_hold_em_init");
	}
}

void table_response_hold_em_betting(hold_em_t *hold_em,
                            		player_t *active_player,
                                	action_request_t *data,
                                	int stage) {
    json_t *action, *active, *comm_cards, *root;
    player_t *player;
	char *response, card_string[4];
	size_t len;
	int i;
	
	action = json_array();
	json_array_append_new(action, json_integer(active_player->position));
	json_array_append_new(action, json_string(active_player->name));
	switch (data->kind) {
		case ACTION_DATA_FOLD:
			json_array_append_new(action, json_string("fold"));
			break;
		case ACTION_DATA_CALL:
			json_array_append_new(action, json_string("call"));
			break;
		case ACTION_DATA_RAISE:
			json_array_append_new(action, json_string("6"));
			break;
		default:
			json_array_append_new(action, json_string("error"));
	}
	json_array_append_new(action, json_integer(active_player->odd));
	json_array_append_new(action, json_integer(active_player->stack));
	
    active = json_pack("{s:i,s:s}",
				       "position", player_list_get(hold_em->players, 0)->position,
				       "name", player_list_get(hold_em->players, 0)->name);

    switch (stage) {
        case HOLD_EM_STAGE_END_FOLD:
        case HOLD_EM_STAGE_END_SHOWDOWN:
        case HOLD_EM_STAGE_COMMON:
            comm_cards = NULL;
            break;
        case HOLD_EM_STAGE_FLOP:
            comm_cards = json_array();
            
            card_to_string(hold_em->comm_cards->cards, card_string);
            json_array_append_new(comm_cards, json_string(card_string));
            
            card_to_string(hold_em->comm_cards->cards + 1, card_string);
            json_array_append_new(comm_cards, json_string(card_string));
            
            card_to_string(hold_em->comm_cards->cards + 2, card_string);
            json_array_append_new(comm_cards, json_string(card_string));
            break;
        case HOLD_EM_STAGE_TURN:
            comm_cards = json_array();
            
            card_to_string(hold_em->comm_cards->cards + 3, card_string);
            json_array_append_new(comm_cards, json_string(card_string));
            break;
        case HOLD_EM_STAGE_RIVER:
            comm_cards = json_array();
            
            card_to_string(hold_em->comm_cards->cards + 4, card_string);
            json_array_append_new(comm_cards, json_string(card_string));
            break;
        default:
            puts("error in table_response_hold_em_mid");
            return;
    }

    if (comm_cards) {
	    root = json_pack("{s:{s:o,s:o,s:o}}",
					     "betting",
					     "action", action,
					     "active", active,
					     "flop", comm_cards);
    } else {
 	    root = json_pack("{s:{s:o,s:o}}",
					     "betting",
					     "action", action,
					     "active", active);
    }
					 
	response = json_dumps(root, JSON_COMPACT);
	len = strlen(response);
	for (i = 0; i < hold_em->curr_players; i++) {
		player = player_list_get(hold_em->players, i);
		if (write(player->conn, response, len) != len)
			puts("error in table_response_hold_em_mid");
	}
}

void table_response_hold_em_showdown(hold_em_t *hold_em) {
	json_t *cards, *reveal, *root;
	player_t *player;
	char *response, card_string[4];
	size_t len;
	int i;

	reveal = json_array();
	for (i = 0; i < player_list_count(hold_em->players); i++) {
		player = player_list_pop(hold_em->players);
		cards = json_array();
		card_to_string(player->hand, card_string);
		json_array_append_new(cards, json_string(card_string));
		card_to_string(player->hand + 1, card_string);
		json_array_append_new(cards, json_string(card_string));
		json_array_append_new(reveal, json_pack("[so]", player->name, cards));
	}
	root = json_pack("{s:o}", "reveal", reveal);
	response = json_dumps(root, JSON_COMPACT);
	len = strlen(response);
	for (i = 0; i < hold_em->curr_players; i++) {
		player = player_list_get(hold_em->players, i);
		if (write(player->conn, response, len) != len)
			puts("error in table_response_hold_em_reveal");
	}
}

void table_response_hold_em_winners(hold_em_t *hold_em, const char **name, int gain) {
	json_t *winners, *root;
	player_t *player;
	char *response;
	size_t len;
	int i;

	winners = json_array();
	for (i = 0; name[i] != NULL; i++) {
		json_array_append_new(winners, json_pack("{s:s,s:i}",
												 "name", name[i],
												 "gain", gain));
	}
	root = json_pack("{s:o}", "winners", winners);
	response = json_dumps(root, JSON_COMPACT);
	len = strlen(response);
	for (i = 0; i < hold_em->curr_players; i++) {
		player = player_list_get(hold_em->players, i);
		if (write(player->conn, response, len) != len)
			puts("error in table_response_hold_em_winners");
	}
}

void table_response_hold_em_reset(hold_em_t *hold_em) {
    player_t *player;
    char *response;
    size_t len;
    int i;
    
    response = json_dumps(json_pack("{s:i}", "reset", 0), JSON_COMPACT);
    len = strlen(response);
    for (i = 0; i < hold_em->curr_players; i++) {
        player = player_list_get(hold_em->players, i);
        if (write(player->conn, response, len) != len)
            puts("error in table_response_hold_em_reset");
    }
}

#endif
