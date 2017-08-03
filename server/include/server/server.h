#ifndef SERVER_H
#define SERVER_H

#define HOST "127.0.0.1"
//#define HOST "5.228.57.21"
#define PORT 5000
#define BUFF_SIZE 1024

typedef struct client client_t;
typedef struct server server;
typedef struct account account_t;
//typedef struct holdem_table holdem_table_t;
//typedef struct table_stud table_stud_t;

typedef struct abs_table {
	int game_type;
	int id;
	void *game;
} abs_table_t;

typedef struct server {
	int socket;
	account_t *acc;
	abs_table_t *abs_tables[4];
} server;

void server_init(server*);
void server_run(server*);

#endif