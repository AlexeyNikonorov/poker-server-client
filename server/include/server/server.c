#include "server.h"
#include "client.h"
#include "response/server_parser.h"
#include "response/server_response.h"

#include "../account/account.h"
#include "../holdem/holdem_table.h"
#include "../table_stud/table_stud.h"

//#include "../db/db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

static int server_new_socket(const char*, int);
static int server_run_async(void* (*worker)(void*), void* args);
static void *server_add(void*);
static void *server_listener(server*, client_t*);
static void *server_handler(server*, client_t*, const char*, ssize_t);
static void *server_disconnect(server*, client_t*);
static void *server_handle_auth(server*, client_t*, auth_request_t*);
static void *server_handle_account(server*, client_t*, account_request_t*);
static void *server_handle_join(server*, client_t*, join_request_t*);

void server_init(server *srv) {
	srv->socket = server_new_socket(HOST, PORT);
	srv->acc = account_alloc(srv);

	srv->abs_tables[0] = (abs_table_t*) holdem_table_alloc(0, 4, 1.0, 2.0);
	srv->abs_tables[1] = (abs_table_t*) holdem_table_alloc(1, 4, 1.0, 2.0);
	srv->abs_tables[2] = (abs_table_t*) table_stud_alloc(2, 4, 2.0, 4.0, 1.0, 0.5);
	srv->abs_tables[3] = NULL;
}

void server_run(server *srv) {
	struct { server *srv; client_t *client; } *listener_args;
	struct sockaddr_in addr;
	size_t size = sizeof(addr);
	int conn;
	
	if (srv->socket < 0)
		return;
	
	listener_args = malloc(sizeof(struct { server *srv; client_t *client; }));
	if (listener_args == NULL)
	    return;
    
	listener_args->srv = srv;
	
	for (;;) {
		conn = accept(srv->socket, (struct sockaddr*) &addr, (socklen_t*) &size);
		if (conn < 0)
			continue;
		listener_args->client = client_alloc(conn);
		if (listener_args->client == NULL)
			continue;
		server_run_async(server_add, (void*) listener_args);
	}
}

static int server_new_socket(const char* host, int port) {
    struct sockaddr_in addr;
    int s;
	s = socket(PF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return -1;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, (struct sockaddr*) &addr, sizeof addr) < 0)
		return -1;
	if (listen(s, 5) < 0)
		return -1;
	return s;
}

static int server_run_async(void* (*worker)(void*), void* args) {
	pthread_t new_thread;
	return pthread_create(&new_thread, NULL, worker, args);
}

static void *server_add(void *args) {
    struct { server *srv; client_t *client; } *_args = args;
    return server_listener(_args->srv, _args->client);
}

static void *server_listener(server *srv, client_t *client) {
    char request[BUFF_SIZE];
    ssize_t len;
	
	len = read(client->conn, request, BUFF_SIZE);
	
	if (len < 1)
		return server_disconnect(srv, client);
	
    return server_handler(srv, client, request, len);
}

static void *server_handler(server *srv, client_t *client, const char *request, ssize_t len) {
    server_parser_t parser;
	
	printf("srv: %s\n", request);
	
	switch (server_parser_parse(&parser, request, len)) {
		case SERVER_PARSER_AUTH:
			return server_handle_auth(srv, client, &parser.auth);
			
		case SERVER_PARSER_ACCOUNT:
		    return server_handle_account(srv, client, &parser.acc);
		    
		case SERVER_PARSER_JOIN:
		    return server_handle_join(srv, client, &parser.join);
		    
		case SERVER_PARSER_ERROR:
		    puts("error in server_handler");
		    
		default:
			return server_listener(srv, client);
	}
}

static void *server_disconnect(server *srv, client_t *client) {
	puts("disconnected from server");
	close(client->conn);
	client_free(client);
	return NULL;
}

static void *server_handle_auth(server *srv, client_t *client, auth_request_t *request) {
	static int id_gen = 0;
	//if (DBVerify(request->login, request->pwd, client) == -1) {
	//	DBNewAccount(request->login, request->pwd, client);
	//	DBVerify(request->login, request->pwd, client);
	//}
	client->id = ++id_gen;
	client->name = request->login;
	server_response_auth(client->conn, client->id, client->name);
	return server_listener(srv, client);
}

static void *server_handle_account(server *srv, client_t *client, account_request_t *request) {
    client->id = request->id;
    client->name = request->name;
    return account_add(srv->acc, client);
}

static void *server_handle_join(server *srv, client_t *client, join_request_t *request) {
	abs_table_t *table;

	client->id = request->client_id;
	client->name = request->name;
	switch (request->game_type) {
		case 1:
			table = srv->abs_tables[request->table_id];
			if (table->game_type != 1) {
				puts("error in server_handle_join");
				return server_listener(srv, client);
			}
			printf("srv: add '%s'\n", client->name);
			return holdem_table_add((holdem_table_t*) table, client);
		
		case 2:
			table = srv->abs_tables[request->table_id];
			if (table->game_type != 2) {
				puts("error in server_handle_join");
				return server_listener(srv, client);
			}
			return table_stud_add((table_stud_t*) table, client);
		
		default:
			puts("error in server_handle_join");
			return server_listener(srv, client);
	}
}
