#ifndef STUD_PLAYER_H
#define STUD_PLAYER_H

#include "card.h"
#include "deck.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

enum { STUD_UPCARDS = 4,
	   STUD_DOWNCARDS = 3, };

enum { PLAYER_STATUS_OFFLINE = 0,
       PLAYER_STATUS_NEW = 1,
       PLAYER_STATUS_FOLD = 2,
	   PLAYER_STATUS_COMMON = 4,
	   PLAYER_STATUS_ACTIVE = 8 };

typedef struct player {
    int conn;
    int id;
    const char *name;
    int client_status;
    int status;
    int position;
    float bet;
    float stack;
    card_t upcards[STUD_UPCARDS];
    card_t downcards[STUD_DOWNCARDS];
} player_t;

static player_t *player_alloc(float bet, float stack) {
	player_t *player;

    player = malloc(sizeof(player_t));
	if (player == NULL)
		return NULL;
	player->name = NULL;
	player->status = PLAYER_STATUS_COMMON;
	player->bet = bet;
	player->stack = stack;
	memset(player->upcards, 0, STUD_UPCARDS*sizeof(card_t));
	memset(player->downcards, 0, STUD_DOWNCARDS*sizeof(card_t));
	return player;
}

static void player_free(player_t *player) {
    close(player->conn);
	free(player);
}

static void player_init(player_t *player) {
    player->status = PLAYER_STATUS_COMMON;
    memset(player->upcards, 0, STUD_UPCARDS*sizeof(card_t));
    memset(player->downcards, 0, STUD_DOWNCARDS*sizeof(card_t));
}

static float player_make_bet(player_t *player, float bet) {
	if (bet < player->stack) {
		player->bet += bet;
		player->stack -= bet;
		return bet;
	} else {
		int tmp_stack = player->stack;
		player->bet += tmp_stack;
		player->stack = 0;
		return tmp_stack;
	}
}

static int player_count_upcards(player_t *player) {
    int i;

    for (i = 0; i < STUD_UPCARDS; i++)
        if (player->upcards[i].v == 0)
            return i;
    return STUD_UPCARDS;
}

static int player_count_downcards(player_t *player) {
    int i;

    for (i = 0; i < STUD_DOWNCARDS; i++)
        if (player->downcards[i].v == 0)
            return i;
    return STUD_DOWNCARDS;
}

static void player_pick_upcard(player_t *player, deck_t *deck) {
    int count;

    count = player_count_upcards(player);
    if (count == STUD_UPCARDS) {
        puts("error in player_pick_upcard");
        return;
    }
	deck_throw_card(deck, player->upcards+count);
}

static void player_pick_downcard(player_t *player, deck_t *deck) {
    int count;

    count = player_count_downcards(player);
    if (count == STUD_DOWNCARDS) {
        puts("error in player_pick_downcard");
        return;
    }
	deck_throw_card(deck, player->downcards+count);
}

static void player_print(player_t *player) {
    char s[4];
    int i;

    printf("%s: %.2f/%.2f\n", player->name, player->bet, player->stack);
    printf("  u:[");
    for (i = 0; i < STUD_UPCARDS; i++) {
        if (player->upcards[i].v == 0) {
            printf("]");
            break;
        } else if (i != 0) {
            printf(" ");
        }
        card_to_string(player->upcards+i, s);
        printf("%s", s);
        if (i == STUD_UPCARDS-1)
            printf("]");
    }

    printf(" d:[");
    for (i = 0; i < STUD_DOWNCARDS; i++) {
        if (player->downcards[i].v == 0) {
            printf("]\n");
            break;
        } else if (i != 0) {
            printf(" ");
        }
        card_to_string(player->downcards+i, s);
        printf("%s", s);
        if (i == STUD_DOWNCARDS-1)
            printf("]\n");
    }
}

#endif
