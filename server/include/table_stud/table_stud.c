#include "table_stud.h"
#include "../stud/stud.h"
#include "response/table_stud_parser.h"
#include "response/table_stud_response.h"

//#include "../db/db.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    table_stud_t *table_stud;
    player_t *player;
    const char *request;
    ssize_t len;
} handler_args_t;

static void *table_stud_listener(table_stud_t*, player_t*);
static void *table_stud_disconnect(table_stud_t*, player_t*);
static void *table_stud_handler(void*);
static void table_stud_handle_offline(table_stud_t*, player_t*);
static void table_stud_handle_new_player(table_stud_t*, player_t*, new_player_request_t*);
static void table_stud_handle_action(table_stud_t*, player_t*, action_request_t*);

table_stud_t *table_stud_alloc(int id, int max_players, float sb, float bb, float bring_in, float ante) {
    table_stud_t *table_stud;
    
    table_stud = malloc(sizeof(table_stud_t));
    if (table_stud == NULL) {
        return NULL;
    }
    
    table_stud->game_type = 2;
    table_stud->id = id;
    table_stud->game = stud_alloc(max_players, sb, bb, bring_in, ante);
    if (table_stud->game == NULL) {
        free(table_stud);
        return NULL;
    }
    return table_stud;
}

void *table_stud_add(table_stud_t *table_stud, client_t *client) {
    player_t *player;

    if (client == NULL) {
        puts("error in table_stud_add (1)");
	    return NULL;
    }

    player = player_list_get_by_name(table_stud->game->players, client->name);
    if (player != NULL) {
        printf("table_stud: player '%s' reconnected\n", client->name);
        client_free(client);
        player->client_status = CLIENT_STATUS_ONLINE;
        return table_stud_listener(table_stud, player);
    }

    player = player_alloc(0, 0);
    if (player == NULL) {
        puts("error in table_stud_add (2)");
        return NULL;
    }

    player->conn = client->conn;
    player->id = client->id;
    player->name = client->name;
    player->status = PLAYER_STATUS_NEW;
    client_free(client);
    return table_stud_listener(table_stud, player);
}

void table_stud_data(table_stud_t *table_stud, table_stud_data_t *td) {
    td->id = table_stud->id;
    td->game = 2;
    td->curr_players = table_stud->game->curr_players;
    td->max_players = table_stud->game->max_players;
}

static void *table_stud_listener(table_stud_t *table_stud, player_t *player) {
    char request[BUFF_SIZE];
	handler_args_t hargs;

    memset(request, 0, BUFF_SIZE);

	hargs.table_stud = table_stud;
	hargs.player = player;
	hargs.request = request;
	
	hargs.len = read(player->conn, request, BUFF_SIZE);
	
	if (hargs.len < 1) {
		return table_stud_disconnect(table_stud, player);
    }

    printf("table_stud: %s\n", request);
    table_stud_handler(&hargs);
    return table_stud_listener(table_stud, player);
}

static void *table_stud_disconnect(table_stud_t *table_stud, player_t *player) {
	printf("table_stud: player '%s' disconnected\n", player->name);
	close(player->conn);
	if (player->status == PLAYER_STATUS_NEW) {
		player_list_remove(table_stud->game->players, player);
		return NULL;
	}
	player->client_status = CLIENT_STATUS_OFFLINE;
	table_stud_handle_offline(table_stud, player);
	return NULL;
}

static void *table_stud_handler(void *args) {
    handler_args_t *handler_args;
    table_stud_parser_t parser;

    handler_args = args;

    switch (table_stud_parser_parse(&parser, handler_args->request, handler_args->len)) {
        case TABLE_STUD_PARSER_NEW_PLAYER:
            table_stud_handle_new_player(handler_args->table_stud, handler_args->player, &parser.new_player_request);
            return NULL;

        case TABLE_STUD_PARSER_ACTION:
        	table_stud_handle_action(handler_args->table_stud, handler_args->player, &parser.action_request);
        	return NULL;

        case TABLE_STUD_PARSER_ERROR:
            puts("error in table_stud_handler");

        default:
            return NULL;
    }
}

static void table_stud_handle_new_player(table_stud_t *table_stud, player_t *player, new_player_request_t *request) {
    //client_t client;
    //if (DBFind(request->id, &client) == -1) {
    //    puts("error");
    //}

    //if (DBUpdateBankroll(&client, 100) == -1) {
    //    puts("error 2");
    //}

	player->id = request->id;
	player->name = request->name;
	player->stack = (float) request->stack;

	if (stud_add_player(table_stud->game, player) == 2) {
		stud_arrange(table_stud->game);
	} else {
		stud_response_betting(table_stud->game);
	}
}

static void table_stud_handle_action(table_stud_t *table_stud, player_t *player, action_request_t *request) {
    player_t *next;

	stud_handle_action(table_stud->game, player, (stud_action_t*) request);
    next = player_list_get(table_stud->game->players, 0);
    if (next->client_status == PLAYER_STATUS_OFFLINE)
        table_stud_handle_offline(table_stud, next);
}

static void table_stud_handle_offline(table_stud_t *table_stud, player_t *player) {
    stud_action_t action;

    if (player->client_status != CLIENT_STATUS_OFFLINE) {
        return;
    }
    
    if (table_stud->game->curr_players < 2) {
        player_list_remove(table_stud->game->players, player);
        table_stud->game->curr_players--;
        return;
    }
    
    if (player->status != PLAYER_STATUS_ACTIVE) {
        return;
    }
    
    if (player->bet == table_stud->game->betting->high_bet) {
        action.kind = STUD_ACTION_CALL;
    } else if (player->bet < table_stud->game->betting->high_bet) {
        action.kind = STUD_ACTION_FOLD;
    } else {
        puts("error in table_stud_handle_offline");
    }
    
    puts("act for offline player");
    sleep(1);
    stud_handle_action(table_stud->game, player, &action);
}