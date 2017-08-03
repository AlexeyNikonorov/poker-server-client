#ifndef SERVER_PARSER_H
#define SERVER_PARSER_H

#include <jansson.h>

typedef struct auth_request {
	char *login;
	char *pwd;
} auth_request_t;

typedef struct account_request {
    int id;
    char *name;
} account_request_t;

typedef struct join_request {
	int game_type;
	int table_id;
	int client_id;
	char *name;
} join_request_t;

typedef union server_parser {
	auth_request_t auth;
	join_request_t join;
	account_request_t acc;
} server_parser_t;

enum { SERVER_PARSER_ERROR = -1,
       SERVER_PARSER_AUTH = 0,
	   SERVER_PARSER_JOIN = 1,
	   SERVER_PARSER_ACCOUNT = 2, };

int server_parser_parse(server_parser_t *parser, const char *data, ssize_t len) {
    json_t *root, *request;
	json_error_t error;
	
	root = json_loadb(data, len, 0, &error);
	if (root == NULL)
		return SERVER_PARSER_ERROR;
	
	if (json_unpack(root, "{s:o}", "auth", &request) == 0)
		if (json_unpack(request, "{s:s,s:s}",
								 "email", &parser->auth.login,
								 "pwd", &parser->auth.pwd) == 0)
			return SERVER_PARSER_AUTH;
	
	if (json_unpack(root, "{s:o}", "account", &request) == 0)
        if (json_unpack(request, "{s:i,s:s}",
                				 "id", &parser->acc.id,
            					 "name", &parser->acc.name) == 0)
            return SERVER_PARSER_ACCOUNT;
	
	if (json_unpack(root, "{s:o}", "join", &request) == 0)
		if (json_unpack(request, "{s:i,s:i,s:i,s:s}",
								 "game_type", &parser->join.game_type,
								 "table_id", &parser->join.table_id,
								 "id", &parser->join.client_id,
								 "name", &parser->join.name) == 0)
			return SERVER_PARSER_JOIN;
    
	return SERVER_PARSER_ERROR;
}

#endif
