#ifndef SERVER_RESPONSE_H
#define SERVER_RESPONSE_H

#include <jansson.h>
#include <unistd.h>
#include <string.h>

void server_response_auth(int conn, int id, const char *name) {
    json_t *root;
    char *response;
    size_t len;
    
	root = json_pack("{s:{s:s,s:i,s:i,s:s}}",
					 "auth",
					 "status", "confirmed",
					 "id", id,
					 "bankroll", 5000,
					 "name", name);
	
	response = json_dumps(root, JSON_COMPACT);
	len = strlen(response);
	if (write(conn, response, len) != len)
		puts("write error in server_response_auth");

	free(response);
}

#endif
