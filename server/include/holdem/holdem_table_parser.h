#ifndef HOLDEM_TABLE_PARSER_H
#define HOLDEM_TABLE_PARSER_H

#include "holdem_header.h"
#include <jansson.h>

enum {
	HOLDEM_TABLE_PARSER_NEW_PLAYER 	= 1,
	HOLDEM_TABLE_PARSER_ACTION 		= 2,
	HOLDEM_TABLE_PARSER_ERROR 		= -1
};

typedef struct {
	int 	id;
	char 	*name;
	double 	stack;
} holdem_table_new_player_request_t;

typedef struct {
	int 	kind;
	float 	value;
} holdem_table_action_request_t;

typedef union {
	holdem_table_new_player_request_t 	new_player;
	holdem_table_action_request_t 		action;
} holdem_table_parser_t;

int holdem_table_parser_parse(holdem_table_parser_t *parser, char *data, size_t len) {
	json_t 			*root, *request;
	json_error_t 	error;

	root = json_loadb(data, len, 0, &error);
	if (root == NULL)
		return HOLDEM_TABLE_PARSER_ERROR;

	if (json_unpack(root, "{s:o}", "new_player", &request) == 0)
		if (json_unpack(request, "{s:i,s:s,s:f}",
								 "id", &parser->new_player.id,
								 "name", &parser->new_player.name,
								 "stack", &parser->new_player.stack) == 0)
			return HOLDEM_TABLE_PARSER_NEW_PLAYER;

	if (json_unpack(root, "{s:o}", "action", &request) == 0)
		if (json_unpack(request,  "{s:i,s:f}",
								  "kind", &parser->action.kind,
								  "value", &parser->action.value) == 0)
			return HOLDEM_TABLE_PARSER_ACTION;

	return HOLDEM_TABLE_PARSER_ERROR;
}

#endif