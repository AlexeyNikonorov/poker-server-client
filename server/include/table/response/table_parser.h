#ifndef TABLE_PARSER_H
#define TABLE_PARSER_H

#include <jansson.h>
#include <string.h>

typedef struct new_player_request {
	int id;
	const char* name;
	int stack;
} new_player_request_t;

typedef struct action_request {
	const char* str;
	int kind;
	int val;
} action_request_t;

typedef union table_parser {
	new_player_request_t new_player;
	action_request_t action;
} table_parser_t;

enum { ACTION_DATA_FOLD = 0,
	   ACTION_DATA_CALL = 1,
	   ACTION_DATA_RAISE = 2, };

enum { TABLE_PARSER_ERROR = -1,
       TABLE_PARSER_NEW_PLAYER = 0,
	   TABLE_PARSER_ACTION = 1, };

int table_parser_parse(table_parser_t* p, const char* data, ssize_t len) {
    json_t *root, *request;
	json_error_t error;
	
	root = json_loadb(data, len, 0, &error);
	if (root == NULL)
		return TABLE_PARSER_ERROR;
	
	if (json_unpack(root, "{s:o}", "new_player", &request) == 0)
		if (json_unpack(request, "{s:i,s:i,s:s}",
								 "id", &p->new_player.id,
								 "stack", &p->new_player.stack,
								 "name", &p->new_player.name) == 0)
			return TABLE_PARSER_NEW_PLAYER;
	
	if (json_unpack(root, "{s:o}", "action", &request) == 0) {
		switch (json_typeof(request)) {
		    case JSON_STRING:
                if (strcmp(json_string_value(request), "fold") == 0) {
                    p->action.kind = ACTION_DATA_FOLD;
                    return TABLE_PARSER_ACTION;
                }
                if (strcmp(json_string_value(request), "call") == 0) {
                    p->action.kind = ACTION_DATA_CALL;
                    return TABLE_PARSER_ACTION;
                }
                return TABLE_PARSER_ERROR;
            case JSON_INTEGER:
                p->action.kind = ACTION_DATA_RAISE;
                p->action.val = json_integer_value(request);
                return TABLE_PARSER_ACTION;
            default:
                return TABLE_PARSER_ERROR;
		}
	}
	
	return TABLE_PARSER_ERROR;
}

#endif
