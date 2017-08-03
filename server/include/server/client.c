#include "client.h"
#include <stdlib.h>

client_t *client_alloc(int conn) {
	client_t *clt;
	clt = malloc(sizeof(client_t));
	if (clt == NULL)
		return NULL;
	clt->conn = conn;
	return clt;
}

void client_free(client_t *clt) {
	free(clt);
}
