#include "holdem_table.h"
#include "holdem.h"
#include "holdem_table_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>

void *holdem_table_listener(holdem_table_t *holdem_table, holdem_player_t *player);
void *holdem_table_disconnect(holdem_table_t *holdem_table, holdem_player_t *player);
void *holdem_table_handler(holdem_table_t 	*holdem_table,
						   holdem_player_t 	*player,
						   char 			*request,
						   size_t 			len);
void *holdem_table_handle_new_player(holdem_table_t 					*holdem_table,
									 holdem_player_t 					*player,
									 holdem_table_new_player_request_t 	*request);
void *holdem_table_handle_action(holdem_table_t 				*holdem_table,
								 holdem_player_t 				*player,
								 holdem_table_action_request_t 	*request);

holdem_table_t *holdem_table_alloc(int id, int max_players, float sb, float bb) {
	holdem_table_t *holdem_table;

holdem_on_showdown = holdem_response_showdown;
holdem_on_end = holdem_response_winners;

	holdem_table = malloc(sizeof *holdem_table);
	if (holdem_table == NULL) {
		return NULL;
	}
	holdem_table->game_type = 1;
	holdem_table->id = id;
	holdem_table->game = holdem_alloc(max_players, sb, bb);
	if (holdem_table->game == NULL) {
		free(holdem_table);
		return NULL;
	}
	return holdem_table;
}

void holdem_table_free(holdem_table_t *holdem_table) {
	holdem_free(holdem_table->game);
	free(holdem_table);
}

void holdem_table_data(holdem_table_t *holdem_table, holdem_table_data_t *holdem_table_data) {
	holdem_table_data->game_type = holdem_table->game_type;
	holdem_table_data->id = holdem_table->id;
	holdem_table_data->max_players = holdem_table->game->players->len;
	holdem_table_data->curr_players = holdem_table->game->players->nplayers;
	holdem_table_data->sblind = holdem_table->game->betting->sblind;
	holdem_table_data->bblind = holdem_table->game->betting->bblind;
}

void *holdem_table_add(holdem_table_t *holdem_table, client_t *client) {
	holdem_player_t *player;

	player = holdem_player_alloc();
	player->conn = client->conn;
	player->id = client->id;
	player->name = client->name;
	client_free(client);
	return holdem_table_listener(holdem_table, player);
}

void *holdem_table_listener(holdem_table_t *holdem_table, holdem_player_t *player) {
    char *request;
	ssize_t len;

    request = malloc(512);
	len = read(player->conn, request, 512);
	if (len < 1) {
		free(request);
		return holdem_table_disconnect(holdem_table, player);
    }
printf("table: %s\n", request);
    holdem_table_handler(holdem_table, player, request, len);
    return holdem_table_listener(holdem_table, player);
}

void *holdem_table_disconnect(holdem_table_t *holdem_table, holdem_player_t *player) {
	return NULL;
}

void *holdem_table_handler(holdem_table_t 	*holdem_table,
						   holdem_player_t 	*player,
						   char 			*request,
						   size_t 			len) {
	holdem_table_parser_t parser;
	int request_type;

	request_type = holdem_table_parser_parse(&parser, request, len);
	free(request);
	switch (request_type) {
		case HOLDEM_TABLE_PARSER_NEW_PLAYER:
			return holdem_table_handle_new_player(holdem_table, player, &parser.new_player);
		case HOLDEM_TABLE_PARSER_ACTION:
			return holdem_table_handle_action(holdem_table, player, &parser.action);
		default:
			puts("error in table_handler");
			return NULL;
	}
}

void *holdem_table_handle_new_player(holdem_table_t 					*holdem_table,
									 holdem_player_t 					*player,
									 holdem_table_new_player_request_t 	*request) {
	player->bet = 0.0;
	player->stack = (float) request->stack;
	if (holdem_add_player(holdem_table->game, player) == -1) {
		return NULL;
	}
	if (holdem_table->game->players->nplayers == 2) {
		holdem_arrange(holdem_table->game);
		holdem_response_betting(holdem_table->game);
	} else {
		holdem_response_betting(holdem_table->game);
	}
	return NULL;
}

void *holdem_table_handle_action(holdem_table_t 				*holdem_table,
								 holdem_player_t 				*player,
								 holdem_table_action_request_t 	*request) {
	holdem_action_t action;

	action.kind = request->kind;
	action.value = request->value;
	holdem_handle_action(holdem_table->game, player, &action);
	holdem_response_betting(holdem_table->game);
	return NULL;
}