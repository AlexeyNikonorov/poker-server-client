#ifndef TABLE_STUD_PARSER_H
#define TABLE_STUD_PARSER_H

#include <jansson.h>
#include <string.h>

enum { TABLE_STUD_PARSER_ERROR = -1,
	   TABLE_STUD_PARSER_NEW_PLAYER = 1,
	   TABLE_STUD_PARSER_ACTION = 2,
	   ACTION_REQUEST_BRING_IN = 1,
	   ACTION_REQUEST_FOLD = 2,
	   ACTION_REQUEST_CALL = 4,
	   ACTION_REQUEST_SB = 8,
	   ACTION_REQUEST_BB = 16,
	   ACTION_REQUEST_RAISE = 32, };

typedef struct new_player_request {
	int id;
	const char *name;
	double stack;
} new_player_request_t;

typedef struct action_request {
	int kind;
	float value;
} action_request_t;

typedef union table_stud_parser {
	action_request_t action_request;
	new_player_request_t new_player_request;
} table_stud_parser_t;

int table_stud_parser_parse(table_stud_parser_t *parser, const char *data, ssize_t len) {
	json_t *root, *request;
	json_error_t error;

	root = json_loadb(data, len, 0, &error);
	if (root == NULL)
		return TABLE_STUD_PARSER_ERROR;

	if (json_unpack(root, "{s:o}", "new_player", &request) == 0)
		if (json_unpack(request, "{s:i,s:s,s:f}",
								 "id", &parser->new_player_request.id,
								 "name", &parser->new_player_request.name,
								 "stack", &parser->new_player_request.stack) == 0)
			return TABLE_STUD_PARSER_NEW_PLAYER;

	if (json_unpack(root, "{s:o}", "action", &request) == 0)
		switch (json_typeof(request)) {
			case JSON_STRING:
				if (strcmp(json_string_value(request), "fold") == 0) {
					parser->action_request.kind = ACTION_REQUEST_FOLD;
					return TABLE_STUD_PARSER_ACTION;
				}
				if (strcmp(json_string_value(request), "call") == 0) {
					parser->action_request.kind = ACTION_REQUEST_CALL;
					return TABLE_STUD_PARSER_ACTION;
				}
				if (strcmp(json_string_value(request), "bring_in") == 0) {
					parser->action_request.kind = ACTION_REQUEST_BRING_IN;
					return TABLE_STUD_PARSER_ACTION;
				}
				if (strcmp(json_string_value(request), "sb") == 0) {
					parser->action_request.kind = ACTION_REQUEST_SB;
					return TABLE_STUD_PARSER_ACTION;
				}
				if (strcmp(json_string_value(request), "bb") == 0) {
					parser->action_request.kind = ACTION_REQUEST_BB;
					return TABLE_STUD_PARSER_ACTION;
				}
				return TABLE_STUD_PARSER_ERROR;

			case JSON_REAL:
				parser->action_request.kind = ACTION_REQUEST_RAISE;
				parser->action_request.value = (float) json_real_value(request);
				return TABLE_STUD_PARSER_ACTION;

			default:
				return TABLE_STUD_PARSER_ERROR;
		}

	return TABLE_STUD_PARSER_ERROR;
}

#endif