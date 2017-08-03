#ifndef HAND_CHECKER_H
#define HAND_CHECKER_H

#include "card.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int hand_checker_result[6];

typedef struct {
	int res[6];
	const char *name;
} player_hand;

static void player_hand_print(player_hand *ph) {
    int i;
    printf("%s got ", ph->name);
    printf("[ ");
    printf("%d ", ph->res[0]);
    for (i = 1; i < 6; i++) {
        if (ph->res[i] == 0)
            break;
        printf("%d ", ph->res[i]);
    }
    printf("]\n");
}

typedef struct {
	int n;
	int root;
	int pair;
	int pair2;
	int set;
	int quads;
} same_t;

typedef struct {
	int n;
	int root;
} row_t;

typedef struct {
	int flush;
	int suits[4];
} suited_t;

static int card_cmp(const void *c1, const void *c2) {
	return ((card_t*) c2)->v - ((card_t*) c1)->v;
}

static void same_init(same_t *same) {
	same->n = 1;
	same->root = 0;
	same->pair = 0;
	same->pair2 = 0;
	same->set = 0;
	same->quads = 0;
}

static void same_count(same_t *same, card_t *card) {
	if (++same->n == 2)
		same->root = card->v;
}

static void same_dump(same_t *same) {
	switch (same->n) {
		case 1:
			return;
		case 2:
			if (same->pair == 0)
				same->pair = same->root;
			else if (same->pair2 == 0)
				same->pair2 = same->root;
			break;
		case 3:
			if (same->set == 0)
				same->set = same->root;
			break;
		case 4:
			if (same->quads == 0)
				same->quads = same->root;
			break;
		default:
			break;
	}
	same->n = 1;
	same->root = 0;
}

static void row_init(row_t *row) {
	row->n = 1;
	row->root = 0;
}

static void row_count(row_t *row, card_t *card) {
	if (row->n == 5)
		return;
	if (++row->n == 2)
		row->root = card->v;
}

static void row_dump(row_t *row) {
	if (row->n == 5)
		return;
	row->n = 1;
	row->root = 0;
}

static void suited_init(suited_t *suited) {
	suited->flush = -1;
	memset(suited->suits, 0, 4*sizeof(int));
}

static void suited_count(suited_t *suited, card_t *card) {
	if (++suited->suits[card->s] == 5)
		suited->flush = card->s;
}

static void suited_dump(suited_t *suited, card_t cards[7], int *res) {
	int suit;
	int counter;
	int i;

	suit = suited->flush;
	counter = 0;
	for (i = 0; i < 7; i++) {
		if (cards[i].s == suit) {
			res[counter++] = cards[i].v;
			if (counter == 5)
				return;
		}
	}
}

static int st_flush(suited_t *suited, row_t *row, card_t cards[7]) {
	int counter;
	int i;

	if (suited->flush == -1 || row->n != 5)
		return 0;

	if (row->root == 5) {
		if ((cards[0].v == 14 && cards[0].s == suited->flush)
				|| (cards[1].v == 14 && cards[1].s == suited->flush)
				|| (cards[2].v == 14 && cards[2].s == suited->flush)) {
			counter = 2;
		} else {
			return 0;
		}
	} else {
		counter = 1;
	}

	for (i = 0; i < 7; i++)
		if (cards[i].v == row->root)
			break;

	for (; i < 6; i++) {
		switch (cards[i].v - cards[i+1].v) {
			case 0:
				if (cards[i].s != suited->flush && cards[i+1].s != suited->flush)
					return 0;
				continue;
			case 1:
				if (i == 5 && cards[i+1].s != suited->flush)
					return 0;
				if (cards[i].s != suited->flush)
					return 0;
				if (++counter == 5)
					return 1;
				continue;
		}
	}

	return 0;
}

static void kickers(int *kicker, int pos, int len, card_t cards[7]) {
	int counter;
	int i;

	if (pos == 0) {
		counter = 0;
		for (i = 0; i < 7; i++) {
			kicker[counter++] = cards[i].v;
			if (counter == len)
				return;
		}
	}

	if (pos == 1) {
		int root = kicker[-1];
		counter = 0;
		for (i = 0; i < 7; i++) {
			if (cards[i].v != root) {
				kicker[counter++] = cards[i].v;
				if (counter == len)
					return;
			}
		}
	}

	if (pos == 2) {
		int root1 = kicker[-1];
		int root2 = kicker[-2];
		counter = 0;
		for (i = 0; i < 7; i++) {
			if (cards[i].v != root1 && cards[i].v != root2) {
				kicker[counter++] = cards[i].v;
				if (counter == len)
					return;
			}
		}
	}
}

static void hand_checker(card_t comm_cards[5], card_t hand[2], int res[6]) {
	card_t cards[7];
	same_t same;
	row_t row;
	suited_t suited;
	int i;

    memset(res, 0, 6*sizeof(int));
	memcpy(cards, comm_cards, 5*sizeof(card_t));
	memcpy(cards+5, hand, 2*sizeof(card_t));

	qsort(cards, 7, sizeof(card_t), card_cmp);

	same_init(&same);
	row_init(&row);
	suited_init(&suited);

	for (i = 0; i < 6; i++) {
		suited_count(&suited, &cards[i]);
		switch (cards[i].v - cards[i+1].v) {
			case 0:
				same_count(&same, &cards[i]);
				continue;
			case 1:
				same_dump(&same);
				row_count(&row, &cards[i]);
				continue;
			default:
				row_dump(&row);
				same_dump(&same);
		}
	}

	suited_count(&suited, &cards[6]);
	same_dump(&same);

	if (row.n == 4 && row.root == 5 && cards[0].v == 14)
		row.n = 5;

	if (st_flush(&suited, &row, cards)) {
		res[0] = 8;
		res[1] = row.root;
		return;
	}
	if (same.quads) {
		res[0] = 7;
		res[1] = same.quads;
		kickers(res+2, 1, 1, cards);
		return;
	}
	if (same.set && same.pair) {
		res[0] = 6;
		res[1] = same.set;
		res[2] = same.pair;
		return;
	}
	if (suited.flush != -1) {
		res[0] = 5;
		suited_dump(&suited, cards, res+1);
		return;
	}
	if (row.n == 5) {
		res[0] = 4;
		res[1] = row.root;
		return;
	}
	if (same.set) {
		res[0] = 3;
		res[1] = same.set;
		kickers(res+2, 1, 2, cards);
		return;
	}
	if (same.pair && same.pair2) {
		res[0] = 2;
		res[1] = same.pair;
		res[2] = same.pair2;
		kickers(res+3, 2, 1, cards);
		return;
	}
	if (same.pair) {
		res[0] = 1;
		res[1] = same.pair;
		kickers(res+2, 1, 3, cards);
		return;
	}
	res[0] = 0;
	kickers(res+1, 0, 5, cards);
}

static int cmp_results(player_hand* res, const char* winners[], int len) {
    int __cmp(int res1[6], int res2[6]) {
        int i;
        for (i = 0; i < 6; i++)
            if (res1[i] != res2[i])
                return res1[i] - res2[i];
        return 0;
    }
    int n = 1;
    int best = 0;
    int i, cmp;
    for (i = 1; i < len; i++) {
        cmp = __cmp((res + i)->res, (res + best)->res);
        if (cmp > 0) {
            best = i;
            n = 1;
        } else if (cmp == 0) {
            winners[n++] = (res + i)->name;
        }
    }
    winners[0] = (res + best)->name;
    winners[n] = NULL;
    return n;
}

#endif
