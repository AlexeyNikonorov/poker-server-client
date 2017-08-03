#ifndef CLIENT_H
#define CLIENT_H

enum { CLIENT_STATUS_OFFLINE = 0,
       CLIENT_STATUS_ONLINE = 1, };

typedef struct client {
	int conn;
	int id;
	char *name;
	int bankroll;
} client_t;

client_t *client_alloc(int);
void client_free(client_t*);

#endif