#ifndef RESPONSE_H
#define RESPONSE_H

#include "stud.h"
#include <jansson.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void stud_response_include_players(json_t *response, stud_t *stud) {
	json_t *item, *players, *cards;
	char card_string[4];
	int n_downcards, n_upcards, i;

	players = json_array();

	player_list_foreach(stud->players,
		cards = json_array();

		n_downcards = player_count_downcards(__player);
		n_upcards = player_count_upcards(__player);

		if (n_downcards == STUD_DOWNCARDS) {
			for (i = 0; i < n_downcards-1; i++) {
				json_array_append_new(cards, json_string("facedown"));
			}
		} else {
			for (i = 0; i < n_downcards; i++) {
				json_array_append_new(cards, json_string("facedown"));
			}
		}

		for (i = 0; i < n_upcards; i++) {
			card_to_string(__player->upcards+i, card_string);
			json_array_append_new(cards, json_string(card_string));
		}

		if (n_downcards == STUD_DOWNCARDS) {
			json_array_append_new(cards, json_string("facedown"));
		} 

		item = json_pack("{s:s,s:f,s:f,s:i}",
						 "name", __player->name,
						 "bet", __player->bet,
						 "stack", __player->stack,
						 "state", __player->status);

		if (__player->status == PLAYER_STATUS_ACTIVE) {
			json_object_set_new(item, "betting_options", json_integer(betting_options(stud->betting, __player)));
		}

		if (n_downcards != 0 && n_upcards != 0) {
			json_object_set_new(item, "cards", cards);
		}

		json_array_append_new(players, item);
	);

	json_object_set_new(response, "players", players);
}

static void stud_response_include_private_cards(json_t *response, player_t *player) {
	json_t *cards;
	char card_string[4];
	int n_downcards, i;

	n_downcards = player_count_downcards(player);

	if (n_downcards == 0) {
		json_object_del(response, "private_cards");
		return;
	}

	cards = json_array();

	for (i = 0; i < n_downcards; i++) {
		card_to_string(player->downcards+i, card_string);
		json_array_append_new(cards, json_string(card_string));
	}

	json_object_set_new(response, "private_cards", cards);
}

static void stud_response_betting(stud_t *stud) {
	json_t *response, *root;
	char *response_str;
	size_t len;

	response = json_object();
	root = json_object();

	stud_response_include_players(response, stud);

	player_list_foreach(stud->players,
		stud_response_include_private_cards(response, __player);
		json_object_set(root, "betting", response);
		
		response_str = json_dumps(root, JSON_COMPACT|JSON_REAL_PRECISION(5));
		len = strlen(response_str);

		if (write(__player->conn, response_str, len) != len)
			puts("error in stud_response_betting");
	);

	free(response_str);
}

static void stud_response_end(stud_t *stud, player_t **winners, int n, float gain) {
	json_t *winner, *response, *root;
	char *response_str;
	size_t len;
	int i;

	response = json_array();
	for (i = 0; i < n; i++) {
		json_array_append_new(response, json_pack("{s:s,s:f}",
												  "name", winners[i]->name,
												  "gain", gain));
	}

	root = json_object();
	json_object_set_new(root, "end", response);

	response_str = json_dumps(root, JSON_COMPACT|JSON_REAL_PRECISION(5));
	len = strlen(response_str);

	player_list_foreach(stud->players,
		if (write(__player->conn, response_str, len) != len)
			puts("error in stud_response_end");
	);

	free(response_str);
}

static void stud_response_showdown(stud_t *stud) {
	json_t *cards, *response, *root;
	char *response_str, card_string[4];
	size_t len;
	int i;

	response = json_array();
	player_list_for_active(stud->players,
		cards = json_array();
		for (i = 0; i < STUD_DOWNCARDS; i++) {
			card_to_string(__player->downcards+i, card_string);
			json_array_append_new(cards, json_string(card_string));
		}
		json_array_append_new(response, json_pack("{s:s,s:o}",
												  "name", __player->name,
												  "downcards", cards));
	);

	root = json_object();
	json_object_set_new(root, "showdown", response);

	response_str = json_dumps(root, JSON_COMPACT|JSON_REAL_PRECISION(5));
	len = strlen(response_str);

	player_list_foreach(stud->players,
		if (write(__player->conn, response_str, len) != len)
			puts("error in stud_response_showdown");
	);

	free(response_str);
}

#endif