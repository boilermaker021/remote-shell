CC = gcc
CFLAGS = -g

SERVER_OBJECTS = server.o
HEADERS = 

all: server

server: $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) $(SERVER_OBJECTS) -o server

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o remote-shell