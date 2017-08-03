#ifndef HOLDEM_RESPONSE_H
#define HOLDEM_RESPONSE_H

#include "holdem_header.h"
#include "card.h"
#include "comm_cards.h"
#include "holdem_player.h"
#include "holdem_player_list.h"
#include "holdem_betting.h"
#include <jansson.h>
#include <unistd.h>
#include <string.h>

void holdem_response_betting(holdem_t *holdem) {
	json_t 	*players, *player, *priv_cards, *comm_cards, *response;
	char 	*response_str, card_str[4];
	size_t 	size;
	int 	i;

	response = json_object();
	players = json_array();
	holdem_player_list_foreach(holdem->players,
		player = json_object();
		json_object_set_new(player, "name", json_string(__player->name));
		json_object_set_new(player, "bet", json_real(__player->bet));
		json_object_set_new(player, "stack", json_real(__player->stack));
		json_object_set_new(player, "status", json_integer(__player->status));
		json_array_append_new(players, player);
	);
	json_object_set_new(response, "players", players);

	if (holdem->comm_cards->n != 0) {
		comm_cards = json_array();
		for (i = 0; i < holdem->comm_cards->n; i++) {
			card_to_string(holdem->comm_cards->cards+i, card_str);
			json_array_append_new(comm_cards, json_string(card_str));
		}
		json_object_set_new(response, "comm_cards", comm_cards);
	}

	holdem_player_list_foreach(holdem->players,
		priv_cards = json_array();
		card_to_string(__player->hand, card_str);
		json_array_append_new(priv_cards, json_string(card_str));
		card_to_string(__player->hand+1, card_str);
		json_array_append_new(priv_cards, json_string(card_str));
		json_object_set_new(response, "priv_cards", priv_cards);
		response_str = json_dumps(response, JSON_COMPACT);
		size = strlen(response_str) + 1;
		if (write(__player->conn, response_str, size) != size)
			puts("error in holdem_response_betting");
	);
	free(response_str);
}

void holdem_response_showdown(holdem_t *holdem) {
	json_t 	*cards, *item, *reveal, *response;
	char 	*response_str, card_str[4];
	size_t 	size;

	reveal = json_array();
	holdem_player_list_foractive(holdem->players,
		item = json_object();
		cards = json_array();
		card_to_string(__player->hand, card_str);
		json_array_append_new(cards, json_string(card_str));
		card_to_string(__player->hand+1, card_str);
		json_array_append_new(cards, json_string(card_str));
		json_object_set_new(item, "name", json_string(__player->name));
		json_object_set_new(item, "priv_cards", cards);
		json_array_append_new(reveal, item);
	);
	response = json_pack("{s:o}", "showdown", reveal);
	response_str = json_dumps(response, JSON_COMPACT);
	size = strlen(response_str) + 1;
	holdem_player_list_foreach(holdem->players,
		if (write(__player->conn, response_str, size) != size)
			puts("error in holdem_response_showdown");
	);
	free(response_str);
}

void holdem_response_winners(holdem_t *holdem, holdem_player_t **players, int len, float gain) {
    json_t 	*winners, *response;
    char 	*response_str;
    size_t 	size;
    int 	i;
    
    winners = json_array();
    for (i = 0; i < len; i++) {
        json_array_append_new(winners, json_pack("{s:s,s:f}",
                                                 "name", players[i]->name,
                                                 "gain", gain));
    }
    
    response = json_pack("{s:o}", "winners", winners);
    response_str = json_dumps(response, JSON_COMPACT);
    size = strlen(response_str) + 1;
    holdem_player_list_foreach(holdem->players,
        if (write(__player->conn, response_str, size) != size)
            puts("error in holdem_response_winners");
    );
    free(response_str);
}

void holdem_response_reset(holdem_t *holdem) {
	json_t 	*response;
    char 	*response_str;
    size_t 	size;
    
    response = json_pack("{s:i}", "reset", 0);
    response_str = json_dumps(response, JSON_COMPACT);
    size = strlen(response_str) + 1;
	holdem_player_list_foreach(holdem->players,
        if (write(__player->conn, response_str, size) != size) 
            puts("error in holdem_response_reset");
    );
    free(response_str);
}

#endif