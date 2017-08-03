#include "holdem.h"

#define OPTIONS { \
	holdem_player_t *__p; \
	int __options; \
	__p = holdem_player_list_get(h->players, 0); \
	__options = holdem_betting_options(h->betting, __p); \
	printf("Options for %s: ", __p->name); \
	if (__options & BETTING_OPTION_FOLD) printf("Fold, "); \
	if (__options & BETTING_OPTION_CALL) printf("Call, "); \
	if (__options & BETTING_OPTION_CHECK) printf("Check, "); \
	if (__options & BETTING_OPTION_RAISE) printf("Raise, "); \
	if (__options & BETTING_OPTION_RERAISE) printf("Re-raise, "); \
	putchar('\n'); \
}

#define FOLD { \
	holdem_action_t __a; \
	holdem_player_t *__p; \
	__a.kind = BETTING_OPTION_FOLD; \
	__p = holdem_player_list_get(h->players, 0); \
	if (holdem_handle_action(h, __p, &__a) == -1) printf("error\n"); \
}

#define CALL { \
	holdem_action_t __a; \
	holdem_player_t *__p; \
	__a.kind = BETTING_OPTION_CALL; \
	__p = holdem_player_list_get(h->players, 0); \
	if (holdem_handle_action(h, __p, &__a) == -1) printf("error\n"); \
}

#define CHECK { \
	holdem_action_t __a; \
	holdem_player_t *__p; \
	__a.kind = BETTING_OPTION_CHECK; \
	__p = holdem_player_list_get(h->players, 0); \
	if (holdem_handle_action(h, __p, &__a) == -1) printf("error\n"); \
}

#define RAISE { \
	holdem_action_t __a; \
	holdem_player_t *__p; \
	__a.kind = BETTING_OPTION_RAISE; \
	__a.value = 2.0; \
	__p = holdem_player_list_get(h->players, 0); \
	if (holdem_handle_action(h, __p, &__a) == -1) printf("error\n"); \
}

#define RERAISE { \
	holdem_action_t __a; \
	holdem_player_t *__p; \
	__a.kind = BETTING_OPTION_RERAISE; \
	__a.value = 2.0; \
	__p = holdem_player_list_get(h->players, 0); \
	if (holdem_handle_action(h, __p, &__a) == -1) printf("error\n"); \
}

void on_arrange(holdem_t *holdem) {
	holdem_player_t *player;
	char cardstr[3];
	int i;

	for (i = 0; i < holdem->players->len; i++) {
		player = holdem->players->data[i];
		if (player == NULL) {
			continue;
		}
		switch (player->status) {
			case HOLDEM_PLAYER_STATUS_FOLD:
				printf("%s (f): ", player->name);
				break;
			case HOLDEM_PLAYER_STATUS_COMMON:
				printf("%s    : ", player->name);
				break;
			case HOLDEM_PLAYER_STATUS_ACTIVE:
				printf("%s (a): ", player->name);
				break;
		}
		printf("%.2f/%.2f\n", player->bet, player->stack);
		card_to_string(player->hand+0, cardstr);
		printf("        ");
		printf("%s, ", cardstr);
		card_to_string(player->hand+1, cardstr);
		printf("%s\n", cardstr);
	}
	printf("_____________________________________\n");
}

void on_betting(holdem_t *holdem, holdem_player_t *_player, holdem_action_t *action) {
	holdem_player_t *player;
	char cardstr[4];
	int i;

	for (i = 0; i < holdem->players->len; i++) {
		player = holdem->players->data[i];
		if (player == NULL) {
			continue;
		}
		switch (player->status) {
			case HOLDEM_PLAYER_STATUS_FOLD:
				printf("%s (f): ", player->name);
				printf("%.2f/%.2f\n", player->bet, player->stack);
				card_to_string(player->hand+0, cardstr);
				printf("        ");
				printf("%s ", cardstr);
				card_to_string(player->hand+1, cardstr);
				printf("%s\n", cardstr);
				break;
			case HOLDEM_PLAYER_STATUS_COMMON:
				printf("%s    : ", player->name);
				printf("%.2f/%.2f\n", player->bet, player->stack);
				card_to_string(player->hand+0, cardstr);
				printf("        ");
				printf("%s ", cardstr);
				card_to_string(player->hand+1, cardstr);
				printf("%s\n", cardstr);
				break;
			case HOLDEM_PLAYER_STATUS_ACTIVE:
				printf("%s (a): ", player->name);
				printf("%.2f/%.2f", player->bet, player->stack);
				switch (action->kind) {
					case BETTING_OPTION_FOLD:
						printf(" -> fold\n");
						break;
					case BETTING_OPTION_CALL:
						printf(" -> call\n");
						break;
					case BETTING_OPTION_CHECK:
						printf(" -> check\n");
						break;
					case BETTING_OPTION_RAISE:
						printf(" -> raise (+%.2f)\n", action->value);
						break;
					case BETTING_OPTION_RERAISE:
						printf(" -> re-raise (+%.2f)\n", action->value);
						break;
				}
				card_to_string(player->hand+0, cardstr);
				printf("        ");
				printf("%s ", cardstr);
				card_to_string(player->hand+1, cardstr);
				printf("%s\n", cardstr);
				break;
		}
	}
	printf("_____________________________________\n");
}

void on_stage(holdem_t *holdem) {
	char cardstr[4];
	switch (holdem->betting->stage) {
		case BETTING_STAGE_THIRD_STREET:
			printf("Third Street:\n");
			printf("Community Cards: ");
			card_to_string(holdem->comm_cards->cards+0, cardstr);
			printf("%s ", cardstr);
			card_to_string(holdem->comm_cards->cards+1, cardstr);
			printf("%s ", cardstr);
			card_to_string(holdem->comm_cards->cards+2, cardstr);
			printf("%s\n", cardstr);
			break;
		case BETTING_STAGE_FOURTH_STREET:
			printf("Fourth Street:\n");
			printf("Community Cards: ");
			card_to_string(holdem->comm_cards->cards+0, cardstr);
			printf("%s ", cardstr);
			card_to_string(holdem->comm_cards->cards+1, cardstr);
			printf("%s ", cardstr);
			card_to_string(holdem->comm_cards->cards+2, cardstr);
			printf("%s ", cardstr);
			card_to_string(holdem->comm_cards->cards+3, cardstr);
			printf("%s\n", cardstr);
			break;
		case BETTING_STAGE_FIFTH_STREET:
			printf("Fifth Street:\n");
			printf("Community Cards: ");
			card_to_string(holdem->comm_cards->cards+0, cardstr);
			printf("%s ", cardstr);
			card_to_string(holdem->comm_cards->cards+1, cardstr);
			printf("%s ", cardstr);
			card_to_string(holdem->comm_cards->cards+2, cardstr);
			printf("%s ", cardstr);
			card_to_string(holdem->comm_cards->cards+3, cardstr);
			printf("%s ", cardstr);
			card_to_string(holdem->comm_cards->cards+4, cardstr);
			printf("%s\n", cardstr);
			break;

	}
}

void on_end(holdem_t *holdem, holdem_player_t **winners, int len, float gain) {
	holdem_player_t *player;
	char cardstr[4];
	int i;

	printf("Showdown:\n");
	printf("Community Cards:\n");
	card_to_string(holdem->comm_cards->cards+0, cardstr);
	printf("    %s ", cardstr);
	card_to_string(holdem->comm_cards->cards+1, cardstr);
	printf("%s ", cardstr);
	card_to_string(holdem->comm_cards->cards+2, cardstr);
	printf("%s ", cardstr);
	card_to_string(holdem->comm_cards->cards+3, cardstr);
	printf("%s ", cardstr);
	card_to_string(holdem->comm_cards->cards+4, cardstr);
	printf("%s\n", cardstr);
	printf("Private Cards:\n");
	for (i = 0; i < holdem_player_list_count(holdem->players); i++) {
		player = holdem_player_list_get(holdem->players, i);
		printf("    %s: ", player->name);
		card_to_string(player->hand+0, cardstr);
		printf("%s ", cardstr);
		card_to_string(player->hand+1, cardstr);
		printf("%s | ", cardstr);
		card_to_string(holdem->comm_cards->cards+0, cardstr);
		printf("%s ", cardstr);
		card_to_string(holdem->comm_cards->cards+1, cardstr);
		printf("%s ", cardstr);
		card_to_string(holdem->comm_cards->cards+2, cardstr);
		printf("%s ", cardstr);
		card_to_string(holdem->comm_cards->cards+3, cardstr);
		printf("%s ", cardstr);
		card_to_string(holdem->comm_cards->cards+4, cardstr);
		printf("%s\n", cardstr);
	}
	printf("Winner(s):\n");
	for (i = 0; i < len; i++) {
		printf("    %s (+%.2f)\n", winners[i]->name, gain);
	}
}

holdem_player_t *new_player(char *name, float stack) {
	holdem_player_t *player;

	player = holdem_player_alloc();
	player->name = name;
	player->stack = stack;
	return player;
}

int main() {
	//holdem_on_arrange = &on_arrange;
	//holdem_on_betting = &on_betting;
	//holdem_on_stage = &on_stage;
	//holdem_on_end = &on_end;

	holdem_on_arrange 	= &holdem_response_betting;
	holdem_on_betting 	= &holdem_response_betting;
	holdem_on_stage 	= &holdem_response_betting;
	holdem_on_showdown 	= &holdem_response_showdown;
	holdem_on_end 		= &holdem_response_winners;

	holdem_t *h;
	holdem_player_t *p1, *p2, *p3;
	
	h = holdem_alloc(4, 1.0, 2.0);

	p1 = new_player("p1", 100.0); p1->conn = 1;
	p2 = new_player("p2", 101.0); p2->conn = -1;
	p3 = new_player("p3", 102.0); p3->conn = -1;

	holdem_add_player(h, p1);
	holdem_add_player(h, p2);
	holdem_add_player(h, p3);

	holdem_arrange(h);
	for (;;) {
		int action;
		OPTIONS;
		scanf("%d", &action);
		switch (action) {
			case 1:
				FOLD; break;
			case 2:
				CALL; break;
			case 3:
				CHECK; break;
			case 4:
				RAISE; break;
			default:
				return 0;
		}
	}
	return 0;
}