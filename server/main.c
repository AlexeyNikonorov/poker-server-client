#include "include/server/server.h"

int main() {
	server s;
	server_init(&s);
	server_run(&s);
	return 0;
}