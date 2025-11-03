CC = gcc
CFLAGS = -pthread 

all: mutex parallel_factorial deadlock_demo

mutex: mutex.c
	$(CC) -o mutex mutex.c $(CFLAGS)

parallel_factorial: parallel_factorial.c
	$(CC) -o parallel_factorial parallel_factorial.c $(CFLAGS)

deadlock_demo: deadlock_demo.c
	$(CC) -o deadlock_demo deadlock_demo.c $(CFLAGS)

clean:
	rm -f mutex parallel_factorial deadlock_demo
