#ifndef HOLDEM_PLAYER_LIST_H
#define HOLDEM_PLAYER_LIST_H

#include "holdem_header.h"
#include "card.h"
#include "deck.h"
#include "holdem_player.h"
#include <stdlib.h>

#define holdem_player_list_foreach(pl, code) {                      \
    holdem_player_t *__player;                                      \
    int __i;                                                        \
    for (__i = 0; __i < pl->len; __i++) {                           \
        __player = pl->data[__i];                                   \
        if (__player == NULL)                                       \
            continue;                                               \
        code;                                                       \
    }                                                               \
}

#define holdem_player_list_foractive(pl, code) {                    \
    holdem_player_t *__player;                                      \
    int __i;                                                        \
    for (__i = 0; __i < pl->len; __i++) {                           \
        __player = pl->data[__i];                                   \
        if (__player == NULL)                                       \
            continue;                                               \
        if (__player->status == HOLDEM_PLAYER_STATUS_FOLD)          \
        	continue;                                               \
        code;                                                       \
    }                                                               \
}
/*
typedef struct {
	holdem_player_t **data;
	holdem_player_t *active;
	holdem_player_t *next;
	int pos;
	int len;
	int nplayers;
} holdem_player_list_t;
*/
holdem_player_list_t *holdem_player_list_alloc(int len) {
	holdem_player_list_t *pl;
	int i;

	pl = malloc(sizeof *pl);
	if (pl == NULL) {
		return NULL;
	}
	pl->data = malloc(len*sizeof *pl->data);
	if (pl->data == NULL) {
		free(pl);
		return NULL;
	}
	for (i = 0; i < len; i++) {
		pl->data[i] = NULL;
	}
	pl->active = NULL;
	pl->next = NULL;
	pl->pos = 0;
	pl->len = len;
	pl->nplayers = 0;
	return pl;
}

void holdem_player_list_free(holdem_player_list_t *pl) {
	free(pl->data);
	free(pl);
}

int holdem_player_list_push(holdem_player_list_t *pl, holdem_player_t *player) {
	int i;

	if (pl->nplayers == pl->len) {
		return -1;
	}
	for (i = 0; i < pl->len; i++) {
		if (pl->data[i] != NULL) {
			continue;
		}
		pl->data[i] = player;
		pl->nplayers++;
		return 0;
	}
	return -1;
}

int holdem_player_list_remove(holdem_player_list_t *pl, holdem_player_t *player) {
	int i;

	for (i = 0; i < pl->len; i++) {
		if (pl->data[i] != player) {
			continue;
		}
		pl->data[i] = NULL;
		pl->nplayers--;
		return 0;
	}
	return -1;
}

int holdem_player_list_set_pos(holdem_player_list_t *pl, int position) {
	int counter, i;

	counter = 0;
	for (i = 0; i < pl->len; i++) {
		if (pl->data[i] == NULL) {
			continue;
		}
		if (counter++ == position) {
			pl->active = pl->data[i];
			pl->pos = i;
			return 0;
		}
	}
	return -1;
}

int holdem_player_list_inc_pos(holdem_player_list_t *pl, int step) {
	int counter, i;

	counter = 0;
	for (i = pl->pos; ; i++) {
		if (pl->data[i % pl->len] == NULL) {
			continue;
		}
		if (counter++ == step) {
			pl->active = pl->data[i % pl->len];
			pl->pos = i % pl->len;
			return 0;
		}
	}
	return -1;
}

int holdem_player_list_count(holdem_player_list_t *pl) {
	int counter, i;

	counter = 0;
	for (i = 0; i < pl->len; i++) {
		if (pl->data[i] == NULL) {
			continue;
		}
		if (pl->data[i]->status == HOLDEM_PLAYER_STATUS_FOLD) {
			continue;
		}
		counter++;
	}
	return counter;
}

holdem_player_t *holdem_player_list_get(holdem_player_list_t *pl, int index) {
	holdem_player_t *player;
	int counter, i, idx;

	counter = 0;
	index = index % pl->nplayers;
	for (i = 0; i < pl->len; i++) {
		idx = (pl->pos + i) % pl->len;
		player = pl->data[idx];
		if (player == NULL) {
			continue;
		}
		if (counter++ == index) {
			return player;
		}
	}
	return NULL;
}

holdem_player_t *holdem_player_list_next(holdem_player_list_t *pl) {
	holdem_player_t *player;
	int i, idx;

	for (i = 1; pl->len; i++) {
		idx = (pl->pos + i) % pl->len;
		player = pl->data[idx];
		if (player == NULL) {
			continue;
		}
		if (player->status == HOLDEM_PLAYER_STATUS_FOLD) {
			continue;
		}
		pl->pos = idx;
		player->status = HOLDEM_PLAYER_STATUS_ACTIVE;
		return player;
	}
	return NULL;
}

#endif