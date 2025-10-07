all: process_memory parallel_min_max parallel_sum

process_memory: process_memory.c
	gcc -o process_memory process_memory.c

parallel_min_max: ../../lab3/src/parallel_min_max.c ../../lab3/src/find_min_max.c ../../lab3/src/utils.c
	gcc -o parallel_min_max ../../lab3/src/parallel_min_max.c ../../lab3/src/find_min_max.c ../../lab3/src/utils.c

parallel_sum: parallel_sum.c utils.c
	gcc -pthread -o parallel_sum parallel_sum.c utils.c

clean:
	rm -f process_memory parallel_min_max parallel_sum