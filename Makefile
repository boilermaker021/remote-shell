CC = gcc
CFLAGS = -g

OBJECTS = main.o
HEADERS = 

all: server

server: $(OBJECTS)
	$(CC) $(CFLAGS) main.o -o server

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o remote-shell