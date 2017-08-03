#include "account.h"
#include "../server/server.h"
#include "../server/client.h"
//#include "../table/table.h"
#include "response/account_parser.h"
#include "response/account_response.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

static void account_disconnect(account_t*, client_t*);
static int account_handler(account_t*, client_t*, const char*, ssize_t);
static int account_handle_join(account_t*, client_t*, join_request_t*);
static int account_handle_table_data(account_t*, client_t*, table_data_request_t*);

account_t *account_alloc(server *srv) {
    account_t *acc = malloc(sizeof(account_t));
    if (acc == NULL)
        return NULL;
    acc->srv = srv;
    return acc;
}

void *account_add(account_t *acc, client_t *client) {
    if (acc == NULL || client == NULL)
        return NULL;
    return account_listener(acc, client);
}

void *account_listener(account_t *acc, client_t *client) {
    char request[BUFF_SIZE];
	ssize_t len;
	
	len = read(client->conn, request, BUFF_SIZE);
	if (len < 1)
		account_disconnect(acc, client);
	
	printf("acc: %s\n", request);
	
	if (!account_handler(acc, client, request, len))
		pthread_exit(NULL);
	return account_listener(acc, client);
}

static void account_disconnect(account_t *acc, client_t *client) {
	puts("disconnected from account");
	close(client->conn);
	client_free(client);
	pthread_exit(NULL);
}

static int account_handler(account_t *acc, client_t *client, const char* request, ssize_t len) {
    account_parser_t p;
    switch (account_parser_parse(&p, request, len)) {
        case ACCOUNT_PARSER_JOIN:
            return account_handle_join(acc, client, &p.join);
            
        case ACCOUNT_PARSER_TABLE_DATA:
            return account_handle_table_data(acc, client, &p.table_data);
            
        case ACCOUNT_PARSER_ERROR:
            puts("error in account_handler");
            
        default:
            return 1;
    }
}

static int account_handle_join(account_t *acc, client_t *client, join_request_t *request) {
    account_response_join(client->conn, 1);
    return 1;
}

static int account_handle_table_data(account_t *acc, client_t *client, table_data_request_t *request) {
    account_response_table_data(acc, client);
    return 1;
}
