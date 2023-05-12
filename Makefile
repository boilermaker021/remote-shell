CC = gcc
CFLAGS = -g

OBJECTS = main.o

remote-shell: $(OBJECTS)
	$(CC) $(CFLAGS) main.o -o remote-shell

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o remote-shell