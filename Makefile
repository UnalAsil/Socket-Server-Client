
SERVER_OBJECTS = server.o 
CLIENT_OBJECTS = client.o 
CC=gcc
CFLAGS = -O2 -Wall -pedantic

all: clean server client
	
server: $(SERVER_OBJECTS)
	$(info ************  Building server ************)
	$(CC) $(CFLAGS) $(SERVER_OBJECTS) -o server -lpthread

client: $(CLIENT_OBJECTS)
	$(info ************  Building client ************)
	$(CC) $(CFLAGS) $(CLIENT_OBJECTS) -o client

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean: 
	$(info ************  Cleaning project ************)
	-rm *.o
	-rm server
	-rm client