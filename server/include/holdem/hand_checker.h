#ifndef HAND_CHECKER_H
#define HAND_CHECKER_H

#include "card.h"
#include "deck.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

enum { TWO = 0x1,   THREE = 0x2,   FOUR  = 0x4,   FIVE = 0x8,
	   SIX = 0x10,  SEVEN = 0x20,  EIGHT = 0x40,  NINE = 0x80,
	   TEN = 0x100, JACK  = 0x200, QUEEN = 0x400, KING = 0x800, ACE = 0x1000 };

enum { NOTHING = 0,     PAIR = 1,     TWO_PAIRS = 2,
	   SET = 3,         STRAIGHT = 4, FLUSH = 5,
	   FULL_HOUSE = 6,  QUADS = 7,    STRAIGHT_FLUSH = 8 };

uint16_t kickers_new(uint16_t kickers, int n) {
    uint16_t card, result = 0;
    int counter = 0;

    for (card = ACE; card >= TWO; card >>= 1) {
        if (kickers & card) {
            result |= card;
            if (++counter == n)
                return result;
        }
    }
    return result;
}

uint32_t hand_checker(card_t *cards, int len) {
	static uint16_t map[15] = {0,0,TWO,THREE,FOUR,FIVE,SIX,SEVEN,EIGHT,NINE,TEN,JACK,QUEEN,KING,ACE};
	static uint16_t straights[10] = { TEN|JACK|QUEEN|KING|ACE,
									  NINE|TEN|JACK|QUEEN|KING,
									  EIGHT|NINE|TEN|JACK|QUEEN,
									  SEVEN|EIGHT|NINE|TEN|JACK,
									  SIX|SEVEN|EIGHT|NINE|TEN,
									  FIVE|SIX|SEVEN|EIGHT|NINE,
									  FOUR|FIVE|SIX|SEVEN|EIGHT,
									  THREE|FOUR|FIVE|SIX|SEVEN,
									  TWO|THREE|FOUR|FIVE|SIX,
									  ACE|TWO|THREE|FOUR|FIVE };
	uint8_t values[15], suits[4];
	uint16_t hand = 0;
	uint16_t flush = 0, straight = 0, straight_flush = 0;
	uint16_t pair = 0, twopairs = 0, set = 0, quads = 0;
	uint8_t i, suit;
	uint32_t res = 0;

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
			suit = i;
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
		res |= straight_flush << 13;
		res |= STRAIGHT_FLUSH << 26;
		return res;
	} else if (quads) {
		res |= kickers_new(hand^quads, 1);
		res |= quads << 13;
		res |= QUADS << 26;
		return res;
	} else if (set && pair) {
		res |= pair;
		res |= set << 13;
		res |= FULL_HOUSE << 26;
		return res;
	} else if (flush) {
		res |= flush << 13;
		res |= FLUSH << 26;
		return res;
	} else if (straight) {
		res |= straight << 13;
		res |= STRAIGHT << 26;
		return res;
	} else if (set) {
		res |= kickers_new(hand^set, 2);
		res |= set << 13;
		res |= SET << 26;
		return res;
	} else if (twopairs) {
		res |= kickers_new(hand^twopairs, 1);
		res |= twopairs << 13;
		res |= TWO_PAIRS << 26;
		return res;
	} else if (pair) {
		res |= kickers_new(hand^pair, 3);
		res |= pair << 13;
		res |= PAIR << 26;
		return res;
	} else {
		res |= kickers_new(hand, 5) << 13;
		res |= NOTHING << 26;
		return res;
	}
}

#endif