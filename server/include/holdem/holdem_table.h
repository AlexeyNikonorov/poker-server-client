#ifndef HOLDEM_TABLE_H
#define HOLDEM_TABLE_H

#include "../server/server.h"
#include "../server/client.h"
#include "holdem_header.h"

typedef struct {
	int game_type;
	int id;
	holdem_t *game;
} holdem_table_t;

typedef struct {
	int id;
	int game_type;
	int max_players;
	int curr_players;
	float sblind;
	float bblind;
} holdem_table_data_t;

holdem_table_t *holdem_table_alloc(int id, int max_players, float sb, float bb);
void holdem_table_free(holdem_table_t *holdem_table);
void holdem_table_data(holdem_table_t *holdem_table, holdem_table_data_t *holdem_table_data);
void *holdem_table_add(holdem_table_t *holdem_table, client_t *client);

#endif