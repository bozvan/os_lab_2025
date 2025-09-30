#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Использование: %s <seed> <array_size>\n", argv[0]);
        return 1;
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка при создании процесса");
        return 1;
    }
    if (pid == 0) {
        printf("Дочерний процесс: Запуск sequential_min_max...\n");    
        char *args[] = {"./sequential_min_max", argv[1], argv[2], NULL};
        execvp(args[0], args);
        perror("Ошибка при запуске программы"); // Если execvp вернул управление - произошла ошибка
        exit(1);
    } else {
        printf("Родительский процесс: Ожидание завершения дочернего процесса...\n");
        int status;
        waitpid(pid, &status, 0);  // Ждем завершения дочернего процесса
        if (WIFEXITED(status)) {
            printf("Родительский процесс: Дочерний процесс завершился со статусом %d\n", WEXITSTATUS(status));
        } else {
            printf("Родительский процесс: Дочерний процесс завершился аварийно\n");
        }
    }
    return 0;
}