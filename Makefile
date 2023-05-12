CC = gcc
CFLAGS = -g

OBJECTS = main.o
HEADERS = 

remote-shell: $(OBJECTS)
	$(CC) $(CFLAGS) main.o -o remote-shell

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o remote-shell