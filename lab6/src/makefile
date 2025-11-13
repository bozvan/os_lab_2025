CC = gcc
CFLAGS = -pthread

all: server client

server: server.c common.c common.h
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o server server.c common.c
  
client: client.c common.c common.h
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o client client.c common.c

clean:
	rm -f server client *.o

rebuild: clean all
