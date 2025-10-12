#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <getopt.h>

int k = 0;
int pnum = 1;
int mod = 0;
unsigned long long result = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct ThreadArgs {
    int start;
    int end;
};

void* calculate_partial_factorial(void* args) {
    struct ThreadArgs* thread_args = (struct ThreadArgs*)args;
    unsigned long long partial_result = 1;
    
    for (int i = thread_args->start; i <= thread_args->end; i++) {
        partial_result = (partial_result * i) % mod;
    }
    
    pthread_mutex_lock(&mutex);
    result = (result * partial_result) % mod;
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main(int argc, char** argv) {
    static struct option options[] = {
        {"k", required_argument, 0, 0},
        {"pnum", required_argument, 0, 0},
        {"mod", required_argument, 0, 0},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1) {
        switch (option_index) {
            case 0:
                k = atoi(optarg);
                break;
            case 1:
                pnum = atoi(optarg);
                break;
            case 2:
                mod = atoi(optarg);
                break;
            default:
                printf("Invalid argument\n");
                return 1;
        }
    }
    
    if (k <= 0 || pnum <= 0 || mod <= 0) {
        printf("Usage: %s -k <number> --pnum=<threads> --mod=<modulus>\n", argv[0]);
        printf("All values must be positive integers\n");
        return 1;
    }
    
    if (pnum > k) {
        pnum = k;  // Нельзя иметь больше потоков чем чисел для умножения
    }
    
    printf("Computing %d! mod %d using %d threads\n", k, mod, pnum);
    
    pthread_t threads[pnum];
    struct ThreadArgs thread_args[pnum];
    
    // Распределение работы между потоками
    int numbers_per_thread = k / pnum;
    int remainder = k % pnum;
    int current_start = 1;
    
    for (int i = 0; i < pnum; i++) {
        int numbers_for_this_thread = numbers_per_thread;
        if (i < remainder) {
            numbers_for_this_thread++;
        }
        
        thread_args[i].start = current_start;
        thread_args[i].end = current_start + numbers_for_this_thread - 1;
        current_start += numbers_for_this_thread;
        
        printf("Thread %d: numbers from %d to %d\n", 
               i, thread_args[i].start, thread_args[i].end);
        
        if (pthread_create(&threads[i], NULL, calculate_partial_factorial, 
                          (void*)&thread_args[i]) != 0) {
            printf("Error: pthread_create failed!\n");
            return 1;
        }
    }
    
    for (int i = 0; i < pnum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            printf("Error: pthread_join failed!\n");
            return 1;
        }
    }
    
    pthread_mutex_destroy(&mutex);
    
    printf("Result: %d! mod %d = %llu\n", k, mod, result);
    
    return 0;
}