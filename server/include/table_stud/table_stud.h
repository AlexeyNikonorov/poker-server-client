#ifndef TABLE_STUD_H
#define TABLE_STUD_H

#include "../server/server.h"
#include "../server/client.h"

typedef struct stud stud_t;
typedef struct player player_t;

typedef struct table_stud_data {
    int id;
    int game;
    int sblind;
    int bblind;
    int curr_players;
    int max_players;
} table_stud_data_t;

typedef struct table_stud {
    int game_type;
	int id;
	stud_t *game;
} table_stud_t;

table_stud_t *table_stud_alloc(int id, int max_players, float sb, float bb, float bring_in, float ante);
void *table_stud_add(table_stud_t*, client_t*);
void table_stud_data(table_stud_t*, table_stud_data_t*);

#endif