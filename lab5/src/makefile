CC = gcc
CFLAGS = -pthread -Wall -Wextra

all: parallel_factorial deadlock_demo

parallel_factorial: parallel_factorial.c
	$(CC) $(CFLAGS) -o parallel_factorial parallel_factorial.c

deadlock_demo: deadlock_demo.c
	$(CC) $(CFLAGS) -o deadlock_demo deadlock_demo.c

clean:
	rm -f parallel_factorial deadlock_demo
