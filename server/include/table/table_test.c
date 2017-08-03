#include "table.h"
#include "response/table_parser.h"
#include "../hold_em/hold_em.h"
#include "response/table_response.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>

static void *table_listener(table_t*, client_t*);
static void *table_disconnect(table_t*, player_t*);
static void *table_handler(table_t*, client_t*, char *request, ssize_t len);
static void *table_handle_new_player(table_t*, client_t*, new_player_request_t*);
static void *table_handle_action(table_t*, player_t*, action_request_t*);
static void table_handle_end_fold(table_t*, player_t*, action_request_t*);
static void table_handle_end_showdown(table_t*, player_t*, action_request_t*);
static void table_handle_offline(table_t*, player_t*);

table_t *table_alloc(int id, int sblind, int bblind, int max_players) {
    table_t *table;
    
    table = malloc(sizeof(table_t));
    if (table == NULL) {
        return NULL;
    }
    
    table->id = id;
    table->n_clients = 0;
    table->n_players = 0;
    table->game = hold_em_alloc(sblind, bblind, max_players);
    if (table->game == NULL) {
        free(table);
        return NULL;
    }

    memset(table->spectators, 0, 10*sizeof(client_t*));
    return table;
}

void *table_add(table_t *table, client_t *client) {
	int i;

	for (i = 0; i < 10; i++) {
		if (table->spectators[i] == NULL) {
			table->spectators[i] = client;
			break;
		}
	}
    return table_listener(table, client);
}

void table_data(table_t *table, table_data_t *td) {
    td->id = table->id;
    td->game = 1;
    td->sblind = table->game->sblind;
    td->bblind = table->game->bblind;
    td->curr_players = table->game->curr_players;
    td->max_players = table->game->max_players;
}

static void *table_listener(table_t *table, client_t *client) {
    char request[BUFF_SIZE];
	ssize_t len;

	memset(request, 0, BUFF_SIZE);
	len = read(client->conn, request, BUFF_SIZE);

	if (len < 1) {
		return table_disconnect(table, (player_t*) client);
    }

    return table_handler(table, client, request, len);
}

static void *table_disconnect(table_t *table, player_t *player) {
	printf("table: player '%s' disconnected\n", player->name);
	close(player->conn);
	if (player->status == PLAYER_STATUS_NEW) {
		player_list_remove(table->game->players, player);
		return NULL;
	}
	player->client_status = CLIENT_STATUS_OFFLINE;
	table_handle_offline(table, player);
	return NULL;
}

static void *table_handler(table_t *table, client_t *client, char *request, ssize_t len) {
	table_parser_t p;
	
	printf("table: %s\n", request);
	
	switch (table_parser_parse(&p, request, len)) {
		case TABLE_PARSER_NEW_PLAYER:
			return table_handle_new_player(table, client, &p.new_player);
			
		case TABLE_PARSER_ACTION:
			return table_handle_action(table, (player_t*) client, &p.action);
			
		case TABLE_PARSER_ERROR:
		    puts("error in table_handler");
		    
		default:
			return table_listener(table, client);
	}
}

static void *table_handle_new_player(table_t *table, client_t *client, new_player_request_t *request) {
	player_t *player;

	player = player_alloc(0, 0);
	player->conn = client->conn;
	player->id = client->id;
	player->name = client->name;
	player->odd = 0;
	player->stack = request->stack;

	if (!hold_em_add_player(table->game, player)) {
	    return table_listener(table, client);
    }

	{
		int i;
		for (i = 0; i < 10; i++) {
			if ((table->spectators[i] != NULL) && (table->spectators[i]->id == client->id)) {
				table->spectators[i] = NULL;
				break;
			}
		}
	}
    client_free(client);

	if (table->game->curr_players == 2) {
		hold_em_arrange(table->game);
		table_response_hold_em_init(table->game);
	} else {
        table_response_hold_em_new_player(table->game, player);
	}

	return table_listener(table, (client_t*) player);
}

static void *table_handle_action(table_t *table, player_t *player, action_request_t *request) {
    int stage;

	if (!hold_em_handle_action(table->game, player, request)) {
		return table_listener(table, (client_t*) player);
    }
	
    stage = hold_em_stage(table->game, request);
	switch (stage) {
	    case HOLD_EM_STAGE_COMMON:  
        case HOLD_EM_STAGE_FLOP:
        case HOLD_EM_STAGE_TURN:
        case HOLD_EM_STAGE_RIVER:
	        table_response_hold_em_mid(table->game, player, request, stage);
            break;

        case HOLD_EM_STAGE_END_FOLD:
            table_response_hold_em_mid(table->game, player, request, HOLD_EM_STAGE_END_FOLD);
            table_handle_end_fold(table, player, request);
            break;
            
        case HOLD_EM_STAGE_END_SHOWDOWN:
            table_response_hold_em_mid(table->game, player, request, HOLD_EM_STAGE_END_SHOWDOWN);
            table_handle_end_showdown(table, player, request);
            break;
            
        default:
            puts("error in table_handle_action");
	}

    table_handle_offline(table, player_list_get(table->game->players, 0));
    return table_listener(table, (client_t*) player);
}

static void table_handle_end_fold(table_t *table, player_t *player, action_request_t *request) {
    const char *winners[2];
    int gain;
    
    sleep(3);
    hold_em_end_fold(table->game, winners, &gain);
    table_response_hold_em_winners(table->game, winners, gain);
    
    sleep(3);
    table_response_hold_em_reset(table->game);
    
    sleep(3);
    if (hold_em_arrange(table->game)) {
        table_response_hold_em_init(table->game);
    }
}

static void table_handle_end_showdown(table_t *table, player_t *player, action_request_t *request) {
    const char **winners;
    int gain;
    
    winners = alloca((table->game->curr_players + 1)*sizeof(char*));
    
    sleep(3);
    table_response_hold_em_showdown(table->game);
    
    sleep(3);
    hold_em_end_showdown(table->game, winners, &gain);
    table_response_hold_em_winners(table->game, winners, gain);
    
    sleep(3);
    table_response_hold_em_reset(table->game);
    
    sleep(3);
    if (hold_em_arrange(table->game)) {
        table_response_hold_em_init(table->game);
    }
}

static void table_handle_offline(table_t *table, player_t *player) {
    action_request_t request;

    if (player->client_status != CLIENT_STATUS_OFFLINE) {
    	return;
    }
    
    if (table->game->curr_players < 2) {
        player_list_remove(table->game->players, player);
        table->game->curr_players--;
        return;
    }
    
    if (player->status != PLAYER_STATUS_ACTIVE) {
        return;
    }
    
    if (player->odd == table->game->max_odd) {
        request.kind = ACTION_DATA_CALL;
    } else if (player->odd < table->game->max_odd) {
        request.kind = ACTION_DATA_FOLD;
    } else {
        puts("error in table_handle_offline");
    }
    
    puts("act for offline player");
    sleep(1);
    table_handle_action(table, player, &request);
}
