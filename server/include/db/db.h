#ifndef DB_H
#define DB_H

#include "../server/client.h"

int DBVerify(const char *name, const char *pwd, client_t *client);
int DBFind(int id, client_t *client);
int DBNewAccount(const char *name, const char *pwd, client_t *client);
int DBUpdateBankroll(client_t *client, int value);

#endif