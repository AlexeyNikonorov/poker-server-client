#ifndef HOLDEM_HEADER_H
#define HOLDEM_HEADER_H

/*
#include "card.h"
#include "deck.h"
#include "comm_cards.h"
#include "hand_checker.h"
#include "holdem_player.h"
#include "holdem_player_list.h"
#include "holdem_betting.h"
*/

typedef struct card {
	int s;
	int v;
} card_t;

typedef struct deck {
	const card_t *top;
	const card_t *end;
	card_t cards[52];
} deck_t;

typedef struct {
	card_t cards[5];
	int n;
} comm_cards_t;

typedef struct {
	int conn;
	int id;
	char *name;
	int status;
	int position;
	float bet;
	float stack;
	card_t hand[2];
} holdem_player_t;

typedef struct {
	holdem_player_t **data;
	holdem_player_t *active;
	holdem_player_t *next;
	int pos;
	int len;
	int nplayers;
} holdem_player_list_t;

typedef struct {
	holdem_player_list_t *players;
	float sblind;
	float bblind;
	float high_bet;
	float pot;
	int counter;
	int nraises;
	int stage;
} holdem_betting_t;

typedef struct {
	int kind;
	float value;
} holdem_action_t;

typedef struct holdem {
	deck_t					*deck;
	comm_cards_t 			*comm_cards;
	holdem_player_list_t	*players;
	holdem_betting_t		*betting;
	int 					dealer;
} holdem_t;

#endif