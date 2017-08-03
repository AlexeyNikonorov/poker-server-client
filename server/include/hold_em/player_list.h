#ifndef PLAYER_LIST_H
#define PLAYER_LIST_H

#include "card.h"
#include "deck.h"
#include "player.h"

#include <stdlib.h>
#include <string.h>

#define player_list_foreach(pl, code) {                             \
    int __pos, __i = 0;                                             \
    player_t *__player;                                             \
    for (__pos = 0; __pos < pl->len; __pos++) {                     \
        __player = player_list_get(pl, __pos);                      \
        if (__player->status == PLAYER_STATUS_COMMON                \
                || __player->status == PLAYER_STATUS_ACTIVE) {      \
            code;                                                   \
            __i++;                                                  \
        }                                                           \
    }                                                               \
}

enum { PLAYER_LIST_SUCCESS = 0,
       PLAYER_LIST_FAIL = 1, };

typedef struct player_list {
    player_t **room;
    int pos;
    int len;
    int size;
} player_list_t;

static player_list_t *player_list_alloc(int size) {
    player_list_t *pl;
    int i;
    
    pl = malloc(sizeof(player_list_t));
    if (pl == NULL) {
        puts("error in player_list_alloc (1)");
        return NULL;
    }
    
    pl->room = malloc(size * sizeof(player_t*));
    if  (pl->room == NULL) {
        free(pl);
        puts("error in player_list_alloc (2)");
        return NULL;
    }
    
    pl->pos = 0;
    pl->len = 0;
    pl->size = size;
    for (i = 0; i < size; i++)
        pl->room[i] = NULL;
    return pl;
}

static void player_list_free(player_list_t *pl) {
    int i;
    for (i = 0; i < pl->size; i++)
        player_free(pl->room[i]);
    free(pl->room);
    free(pl);
}

static int player_list_push(player_list_t *pl, player_t *player) {
    int i;

    if (pl->len == pl->size)
        return PLAYER_LIST_FAIL;

    for (i = 0; i < pl->size; i++) {
        if (pl->room[i] == NULL) {
            pl->room[i] = player;
            pl->len++;
            return PLAYER_LIST_SUCCESS;
        }
    }
    return PLAYER_LIST_FAIL;
}

static void player_list_remove(player_list_t *pl, player_t *player) {
    int i;

    for (i = 0; i < pl->size; i++) {
        if (pl->room[i] == player) {
            printf("%s removed\n", player->name);
            player_free(player);
            pl->room[i] = NULL;
            pl->len--;
            return;
        }
    }
    puts("error in player_list_remove");
}

static void player_list_set_pos(player_list_t *pl, int pos) {
    int counter = 0;
    int i;

    for (i = 0; i < pl->size; i++) {
        if (pl->room[i] != NULL) {
            if (counter++ == pos) {
                pl->pos = i;
                return;
            }
        }
    }
    
    puts("error in player_list_set_pos");
}

static void player_list_inc_pos(player_list_t *pl, int step) {
    int counter = 0;
    int i;

    for (i = 0; ; i++) {
        if (pl->room[(pl->pos + i) % pl->size] != NULL) {
            if (counter++ == step) {
                pl->pos = (pl->pos + i) % pl->size;
                return;
            }
        }
    }

    puts("error in player_list_inc_pos");
}

static int player_list_count(player_list_t *pl) {
    int counter = 0;
    int i;

    for (i = 0; i < pl->size; i++) {
        if (pl->room[i] == NULL)
            continue;
        if (pl->room[i]->status == PLAYER_STATUS_COMMON
                || pl->room[i]->status == PLAYER_STATUS_ACTIVE)
            counter++;
    }

    return counter;
}

static player_t *player_list_get(player_list_t *pl, int index) {
    player_t *player;
    int counter = 0;
    int i;

    index = index % pl->len;
    for (i = 0; i < pl->size; i++) {
        player = pl->room[(pl->pos + i) % pl->size];
        if (player != NULL)
            if (counter++ == index)
                return player;
    }

    puts("error in player_list_get");
    return NULL;
}

static player_t *player_list_get_by_name(player_list_t *pl, const char *name) {
    player_t *player;
    int i;

    for (i = 0; i < pl->size; i++)
        if ((player = pl->room[i]) != NULL && strcmp(player->name, name) == 0)
            return player;
    
    return NULL;
}

static player_t *player_list_pop(player_list_t *pl) {
    player_t *player;
    int i;

    for (i = 1; i < pl->size; i++) {
        player = pl->room[(pl->pos + i) % pl->size];
        if (player == NULL)
            continue;
        if (player->status == PLAYER_STATUS_COMMON
                || player->status == PLAYER_STATUS_ACTIVE) {
            pl->pos = (pl->pos + i) % pl->size;
            return player;
        }
    }

    puts("error in player_list_pop");
    return NULL;
}

#endif
