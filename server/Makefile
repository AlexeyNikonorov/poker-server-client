FLAGS   = 
LIBS    = -lpthread -ljansson
SOURCES = include/server/server.c include/server/client.c include/account/account.c include/holdem/holdem_table.c include/table_stud/table_stud.c main.c
TARGET  = a.out

all:
	gcc $(FLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

db:
	gcc $(FLAGS) -o $(TARGET) $(SOURCES) include/db/db.c include/db/sqlite3.o $(LIBS) -ldl

db2:
	gcc $(FLAGS) -o $(TARGET) $(SOURCES) include/sqlite3/sqlite3.o $(LIBS) -ldl
