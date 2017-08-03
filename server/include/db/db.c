#include "db.h"
#include "sqlite3.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum {ID = 0, NAME = 1, PWD = 2, BANKROLL = 3};

static int VerifyCallback(void *client, int n, char **val, char **col) {
	size_t size;

	((client_t*) client)->id = strtol(val[ID], NULL, 10);
	
	size = strlen(val[NAME])*sizeof(char);
	((client_t*) client)->name = malloc(size+1);
	memcpy(((client_t*) client)->name, val[NAME], size);
	((client_t*) client)->name[size] = '\0';
	
	((client_t*) client)->bankroll = strtol(val[BANKROLL], NULL, 10);
	return 0;
}

static int FindCallback(void *client, int n, char **val, char **col) {
	size_t size;

	((client_t*) client)->id = strtol(val[ID], NULL, 10);
	
	size = strlen(val[NAME])*sizeof(char);
	((client_t*) client)->name = malloc(size+1);
	memcpy(((client_t*) client)->name, val[NAME], size);
	((client_t*) client)->name[size] = '\0';
	
	((client_t*) client)->bankroll = strtol(val[BANKROLL], NULL, 10);
	return 0;
}

int DBVerify(const char *name, const char *pwd, client_t *client) {
	static const char *fmt = "SELECT * FROM t WHERE name='%s' AND pwd='%s';";
	char query[74];
	sqlite3 *db;
	
	client->name = NULL;

	sprintf(query, fmt, name, pwd);
	
	sqlite3_open("1.db", &db);
	sqlite3_exec(db, query, VerifyCallback, client, NULL);
	sqlite3_close(db);
	return client->name == NULL ? -1 : 0;
}

int DBFind(int id, client_t *client) {
	static const char *fmt = "SELECT * FROM t WHERE id=%d;";
	char query[36];
	sqlite3 *db;

	client->name = NULL;

	sprintf(query, fmt, id);

	sqlite3_open("1.db", &db);
	sqlite3_exec(db, query, FindCallback, client, NULL);
	sqlite3_close(db);
	return client->name == NULL ? -1 : 0;
}

int DBNewAccount(const char *name, const char *pwd, client_t *client) {
	static const char *fmt = "INSERT INTO t(name,pwd,bankroll) VALUES('%s','%s',%d);";
	char query[100];
	sqlite3 *db;

	sprintf(query, fmt, name, pwd, 5000);

	sqlite3_open("1.db", &db);
	sqlite3_exec(db, query, NULL, NULL, NULL);
	sqlite3_close(db);
	return 0;
}

int DBUpdateBankroll(client_t *client, int value) {
	static const char *fmt = "UPDATE t SET bankroll=%d WHERE id=%d;";
	char query[53];
	sqlite3 *db;

	if (value > client->bankroll)
		return -1;

	client->bankroll -= value;
	sprintf(query, fmt, client->bankroll, client->id);
	
	sqlite3_open("1.db", &db);
	sqlite3_exec(db, query, NULL, NULL, NULL);
	sqlite3_close(db);
	return 0;
}