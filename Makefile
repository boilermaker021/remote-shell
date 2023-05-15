CC = gcc
CFLAGS = -g

SERVER_OBJECTS = server.o
CLIENT_OBJECTS = client.o
HEADERS = 

all: server client

server: $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) $(SERVER_OBJECTS) -o server

client: $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) $(CLIENT_OBJECTS) -o client

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o remote-shell