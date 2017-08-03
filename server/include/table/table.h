#ifndef TABLE_H
#define TABLE_H

#include "../server/server.h"
#include "../server/client.h"

typedef struct hold_em hold_em_t;
typedef struct player player_t;

typedef struct table_data {
    int id;
    int game;
    int sblind;
    int bblind;
    int curr_players;
    int max_players;
} table_data_t;

typedef struct table {
    int game_type;
    int id;
	hold_em_t *game;
} table_t;

table_t *table_alloc(int, int, int, int);
void *table_add(table_t*, client_t*);
void table_data(table_t*, table_data_t*);

#endif
