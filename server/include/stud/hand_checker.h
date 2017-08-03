#ifndef STUD_HAND_CHECKER_H
#define STUD_HAND_CHECKER_H

#include "card.h"
#include "player.h"
#include "player_list.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

enum { TWO = 1 ,  THREE = 2,  FOUR = 4,     FIVE = 8,
       SIX = 16,  SEVEN = 32, EIGHT = 64,   NINE = 128,
       TEN = 256, JACK = 512, QUEEN = 1024, KING = 2048, ACE = 4096 };

enum { NOTHING = 0,     PAIR = 1,     TWO_PAIRS = 2,
       SET = 4,         STRAIGHT = 8, FLUSH = 16,
       FULL_HOUSE = 32, QUADS = 64,   STRAIGHT_FLUSH = 128 };

static void print_cards(card_t *cards, int len) {
    char card_str[4];
    int i;

    printf("[");
    for (i = 0; i < len-1; i++) {
        card_to_string(cards+i, card_str);
        printf("%s ", card_str);
    }
    card_to_string(cards+i, card_str);
    printf("%s]\n", card_str);
}

static void print_result(uint16_t *res) {
    int i;

    printf("[%d", res[0]);
    for (i = 1; i < 6; i++) {
        if (res[i] == 0)
            break;
        printf(" %d", res[i]);
    }
    printf("]\n");
}

static int rescmp(const uint16_t *res1, const uint16_t *res2) {
    int i;

    for (i = 0; i < 2; i++)
        if (res1[i] < res2[i])
            return -1;
        else if (res1[i] > res2[i])
            return 1;
    return 0;
}

static uint16_t kickers_new(uint16_t kickers, uint16_t n) {
    uint16_t card, counter = 0, result = 0;

    for (card = ACE; card >= TWO; card >>= 1) {
        if (kickers & card) {
            result |= card;
            if (++counter == n)
                return result;
        }
    }
    return result;
}

static void hand_checker_new(const card_t *cards, uint16_t len, uint16_t *result) {
    static const uint16_t map[15] = {0,0,TWO,THREE,FOUR,FIVE,SIX,SEVEN,EIGHT,NINE,TEN,JACK,QUEEN,KING,ACE};
    static const uint16_t straights[10] = { TEN|JACK|QUEEN|KING|ACE,
                                            NINE|TEN|JACK|QUEEN|KING,
                                            EIGHT|NINE|TEN|JACK|QUEEN,
                                            SEVEN|EIGHT|NINE|TEN|JACK,
                                            SIX|SEVEN|EIGHT|NINE|TEN,
                                            FIVE|SIX|SEVEN|EIGHT|NINE,
                                            FOUR|FIVE|SIX|SEVEN|EIGHT,
                                            THREE|FOUR|FIVE|SIX|SEVEN,
                                            TWO|THREE|FOUR|FIVE|SIX,
                                            ACE|TWO|THREE|FOUR|FIVE };
    uint16_t values[15], suits[4];
    uint16_t hand = 0;
    uint16_t flush = 0, straight = 0, straight_flush = 0;
    uint16_t pair = 0, twopairs = 0, set = 0, quads = 0;
    uint16_t i;

    memset(values, 0, sizeof values);
    memset(suits, 0, sizeof suits);

    for (i = 0; i < len; i++) {
        hand |= map[cards[i].v];
        values[cards[i].v]++;
        suits[cards[i].s]++;
    }

    for (i = 14; i > 1; i--) {
        if (values[i] == 2) {
            if (pair) {
                if (!twopairs) {
                    twopairs |= pair|map[i];
                }
            } else {
                pair |= map[i];
            }
        } else if (values[i] == 3) {
            if (!set) {
                set |= map[i];
            } else if (!pair) {
                pair |= map[i];
            }
        } else if (values[i] == 4) {
            quads |= map[i];
        }
    }

    for (i = 0; i < 4; i++) {
        if (suits[i] > 4) {
            uint16_t suit = i;
            for (i = 0; i < len; i++) {
                if (cards[i].s == suit) {
                    flush |= map[cards[i].v];
                }
            }
            break;
        }
    }

    for (i = 0; i < 10; i++) {
        if ((hand & straights[i]) == straights[i]) {
            straight = straights[i];
            if (flush) {
                for (; i < 10; i++) {
                    if ((flush & straights[i]) == straights[i]) {
                        straight_flush = straights[i];
                        break;
                    }
                }
            }
            break;
        }
    }

    if (straight_flush) {
        result[0] = STRAIGHT_FLUSH;
        result[1] = straight_flush;
    } else if (quads) {
        result[0] = QUADS;
        result[1] = quads;
        result[2] = kickers_new(hand^quads, 1);
    } else if (set && pair) {
        result[0] = FULL_HOUSE;
        result[1] = set;
        result[2] = pair;
    } else if (flush) {
        result[0] = FLUSH;
        result[1] = flush;
    } else if (straight) {
        result[0] = STRAIGHT;
        result[1] = straight;
    } else if (set) {
        result[0] = SET;
        result[1] = set;
        result[2] = kickers_new(hand^set, 2);
    } else if (twopairs) {
        result[0] = TWO_PAIRS;
        result[1] = twopairs;
        result[2] = kickers_new(hand^twopairs, 1);
    } else if (pair) {
        result[0] = PAIR;
        result[1] = pair;
        result[2] = kickers_new(hand^pair, 3);
    } else {
        result[0] = NOTHING;
        result[1] = kickers_new(hand, 5);
    }
}

static player_t *hand_checker_pick_active_player_bring_in(player_list_t *players) {
    player_t *active_player;
    card_t lowest_card;

    active_player = NULL;
    lowest_card.s = 4;
    lowest_card.v = 15;
    player_list_for_active(players,
        card_t upcard;
        upcard = __player->upcards[0];
        if ((upcard.v < lowest_card.v) || ((upcard.v == lowest_card.v) && (upcard.s < lowest_card.s))) {
            active_player = __player;
            lowest_card = upcard;
        }
    );

    if (active_player == NULL) {
        puts("error in hand_checker_pick_active_player");
        return NULL;
    } else {
        active_player->status = PLAYER_STATUS_ACTIVE;
        player_list_rewind(players);
        return active_player;
    }
}

static player_t *hand_checker_pick_active_player(player_list_t *players, int *high_hand) {
    player_t *active_player;
    uint16_t res1[3], res2[3], *max, *cur;
    int n_upcards;

    active_player = NULL;
    memset(res1, 0, sizeof res1);
    max = res1;
    cur = res2;
    n_upcards = player_count_upcards(player_list_get(players, 0));
    player_list_for_active(players,
        memset(cur, 0, sizeof res1);
        hand_checker_new(__player->upcards, n_upcards, cur);
        if (rescmp(max, cur) < 0) {
            uint16_t *tmp;
            tmp = max;
            max = cur;
            cur = tmp;
            active_player = __player;
        }
    );

    if (active_player == NULL) {
        puts("error in hand_checker_pick_active_player");
        return NULL;
    } else {
        active_player->status = PLAYER_STATUS_ACTIVE;
        player_list_rewind(players);
        *high_hand = max[0];
        return active_player;
    }
}

static int hand_checker_winners(player_list_t *players, player_t **winners) {
    uint16_t res1[3], res2[3], *max, *cur;
    int ties;

    memset(res1, 0, sizeof res1);
    max = res1;
    cur = res2;
    ties = 0;
    player_list_for_active(players,
        card_t cards[7];
        int cmp;
        memcpy(cards, __player->upcards, sizeof __player->upcards);
        memcpy(cards+STUD_UPCARDS, __player->downcards, sizeof __player->downcards);
        memset(cur, 0, sizeof res1);
        hand_checker_new(cards, 7, cur);
        cmp = rescmp(max, cur);
        if (cmp < 0) {
            uint16_t *tmp;
            tmp = max;
            max = cur;
            cur = tmp;
            winners[ties=0] = __player;
        } else if (cmp == 0) {
            winners[++ties] = __player;
        }
    );
    return ties;
}

#endif