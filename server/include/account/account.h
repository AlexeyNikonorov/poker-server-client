#ifndef ACCOUNT_H
#define ACCOUNT_H

enum { ACCOUNT_FAIL = 1,
       ACCOUNT_SUCCESS = 0, };

typedef struct server server;
typedef struct client client_t;

typedef struct account {
    int n_clients;
    server *srv;
} account_t;

account_t *account_alloc(server *srv);
void* account_add(account_t *acc, client_t *client);
void* account_listener(account_t*, client_t*);

#endif
