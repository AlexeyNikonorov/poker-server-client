#ifndef TABLE_STUD_RESPONSE_H
#define TABLE_STUD_RESPONSE_H

#include "../../stud/stud.h"
#include <jansson.h>
#include <string.h>
#include <stdlib.h>

void table_stud_response_new_player(stud_t *stud, player_t *player) {
	json_t *root;
	char *response;
	size_t len;

	root = json_pack("{s:{s:s,s:f}}",
					 "new_player",
					 "name", player->name,
					 "stack", player->stack);
	if (root == NULL) {
		puts("error in table_stud_response_new_player (1)");
		return;
	}

	response = json_dumps(root, JSON_COMPACT);
	if (response == NULL) {
		puts("error in table_stud_response_new_player (2)");
	}

	len = strlen(response);
	player_list_foreach(stud->players,
		if (write(__player->conn, response, len) != len)
			puts("error in table_stud_response_new_player(3)");
	);

	free(response);
}

void table_stud_response_init(stud_t *stud) {
	json_t *root, *players, *cards;
	char *response, card_string[4];
	size_t len;

	players = json_array();
	cards = json_array();

	player_list_foreach(stud->players,
		json_array_append_new(cards, json_string("facedown"));
		json_array_append_new(cards, json_string("facedown"));
		card_to_string(__player->upcards+0, card_string);
		json_array_append_new(cards, json_string(card_string));
		json_array_append_new(players, json_pack("{s:s,s:f,s:f,s:o,s:i}",
												 "name", __player->name,
												 "bet", __player->bet,
												 "stack", __player->stack,
												 "cards", json_deep_copy(cards),
												 "state", __player->status));
		json_array_clear(cards);
	);

	player_list_foreach(stud->players,
		card_to_string(__player->downcards+0, card_string);
		json_array_append_new(cards, json_string(card_string));
		card_to_string(__player->downcards+1, card_string);
		json_array_append_new(cards, json_string(card_string));
		
		root = json_pack("{s:{s:o,s:o}}",
						 "init",
						 "players", players,
						 "facedown_cards", json_deep_copy(cards));
		
		response = json_dumps(root, JSON_COMPACT);
		if (response == NULL) {
			puts("error in table_stud_response_init (1)");
			continue;
		}

		len = strlen(response);
		if (write(__player->conn, response, len) != len) {
			puts("error in table_stud_response_init (2)");
		}
		
		free(response);
		json_array_clear(cards);
	);
}

#endif