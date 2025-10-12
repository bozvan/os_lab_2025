#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

// Функция для потока 1 (захватывает mutex1, потом пытается захватить mutex2)
void* thread1_function(void* arg) {
    (void)arg; // Явно указываем что параметр не используется
    printf("Thread 1: Trying to lock mutex1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Thread 1: Locked mutex1\n");
    
    // Имитация работы
    sleep(1);
    printf("Thread 1: Trying to lock mutex2...\n");
    pthread_mutex_lock(&mutex2);  // DEADLOCK!
    printf("Thread 1: Locked mutex2\n");
    
    // Критическая секция
    printf("Thread 1: Working with both mutexes\n");
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return NULL;
}

// Функция для потока 2 (захватывает mutex2, потом пытается захватить mutex1)
void* thread2_function(void* arg) {
    (void)arg; // Явно указываем что параметр не используется
    printf("Thread 2: Trying to lock mutex2...\n");
    pthread_mutex_lock(&mutex2);
    printf("Thread 2: Locked mutex2\n");
    
    // Имитация работы
    sleep(1);
    printf("Thread 2: Trying to lock mutex1...\n");
    pthread_mutex_lock(&mutex1);  // DEADLOCK!
    printf("Thread 2: Locked mutex1\n");
    
    // Критическая секция
    printf("Thread 2: Working with both mutexes\n");
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    
    printf("=== DEADLOCK DEMONSTRATION ===\n");
    printf("This program will demonstrate a classic deadlock scenario.\n");
    printf("Two threads will try to lock two mutexes in different order.\n\n");
    
    // Создаем потоки
    if (pthread_create(&thread1, NULL, thread1_function, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    
    if (pthread_create(&thread2, NULL, thread2_function, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    
    // Даем потокам время на выполнение
    sleep(3);
    
    printf("\nDEADLOCK DETECTED!\n");
    printf("Threads are blocked waiting for each other's mutexes.\n");
    printf("The program is now hung in deadlock state...\n");
    printf("Press Ctrl+C to terminate the program.\n");
    
    // Программа зависнет здесь из-за deadlock
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("This message will never be printed due to deadlock!\n");
    
    return 0;
}