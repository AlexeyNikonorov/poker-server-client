#ifndef ACCOUNT_PARSER_H
#define ACCOUNT_PARSER_H

#include <jansson.h>

typedef struct table_data_request {
    int id;
    const char *name;
} table_data_request_t;

typedef struct join_request {
	int table_id;
	int client_id;
	const char *name;
} join_request_t;

typedef union account_parser {
    join_request_t join;
    table_data_request_t table_data;
} account_parser_t;

enum { ACCOUNT_PARSER_ERROR = -1,
       ACCOUNT_PARSER_JOIN = 0,
       ACCOUNT_PARSER_TABLE_DATA = 1 };

int account_parser_parse(account_parser_t *p, const char *data, ssize_t len) {
    json_t *root, *request;
	json_error_t error;
	
	root = json_loadb(data, len, 0, &error);
	if (root == NULL)
		return ACCOUNT_PARSER_ERROR;
	
	if (json_unpack(root, "{s:o}",
			"join", &request) == 0) {
		if (json_unpack(request, "{s:i,s:i,s:s}",
				"table_id", &p->join.table_id,
				"id", &p->join.client_id,
				"name", &p->join.name) == 0)
			return ACCOUNT_PARSER_JOIN;
	}
	
	if (json_unpack(root, "{s:o}",
	        "table_data", &request) == 0) {
        if (json_unpack(request, "{s:i,s:s}",
                "id", &p->table_data.id,
                "name", &p->table_data.name) == 0)
            return ACCOUNT_PARSER_TABLE_DATA;
    
    }
	
    return ACCOUNT_PARSER_ERROR;
}

#endif
