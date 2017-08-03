#ifndef ACCOUNT_RESPONSE_H
#define ACCOUNT_RESPONSE_H

#include "../account.h"
#include "account_parser.h"
#include <jansson.h>
#include <unistd.h>
#include <string.h>

#include "../../server/server.h"
#include "../../holdem/holdem_table.h"
#include "../../table_stud/table_stud.h"

void account_response_join(int conn, int status) {
    json_t *root;
    char *response;
    size_t len;
    
    root = json_pack("{s:{s:b}}", "join", "status", status);    
    response = json_dumps(root, JSON_COMPACT);
    len = strlen(response);
    if (write(conn, response, len) != len)
        puts("write error in account_response_join");
    free(response);
}

void account_response_table_data(account_t *acc, client_t *client) {
    json_t *data, *root;
    abs_table_t **tables;
    char *response;
    size_t len;
    int i;

    data = json_array();
	tables = acc->srv->abs_tables;
	for (i = 0; i < 4; i++) {
		if (tables[i] == NULL)
			break;
		switch (tables[i]->game_type) {
			case 1:
				{
					holdem_table_data_t td;
					holdem_table_data((holdem_table_t*) tables[i], &td);
		            json_array_append_new(data, json_pack("{s:i,s:i,s:i,s:i,s:i,s:i}",
		                                                  "id", td.id,
		                                                  "game", 1,
		                                                  "sblind", 1,
		                                                  "bblind", 1,
		                                                  "curr_players", td.curr_players,
		                                                  "max_players", td.max_players));
					break;
				}
			case 2:
				{
					table_stud_data_t td;
					table_stud_data((table_stud_t*) tables[i], &td);
		            json_array_append_new(data, json_pack("{s:i,s:i,s:i,s:i,s:i,s:i}",
		                                                  "id", td.id,
		                                                  "game", 2,
		                                                  "sblind", 1,
		                                                  "bblind", 1,
		                                                  "curr_players", td.curr_players,
		                                                  "max_players", td.max_players));
					break;
				}
		}
	}

    root = json_pack("{s:o}", "table_data", data);
    response = json_dumps(root, JSON_COMPACT);
    len = strlen(response);
    if (write(client->conn, response, len) != len)
        puts("write error in account_response_table_data");
    free(response);
}

#endif
