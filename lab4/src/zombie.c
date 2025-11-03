#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("=== Демонстрация зомби-процесса ===\n\n");
    
    pid_t child_pid = fork();
    
    if (child_pid == 0) {
        printf("Дочерний процесс: PID = %d\n", getpid());
        printf("Завершаю работу... Стану зомби!\n");
        exit(0);  // Завершаемся, но родитель не вызовет wait()
    } else {
        printf("Родительский процесс: PID = %d\n", getpid());
        printf("Создал дочерний процесс: PID = %d\n", child_pid);
        printf("\nЗомби создан! В другом терминале:\n");
        printf("   ps aux | grep %d\n", child_pid);
        printf("Родитель спит 30 секунд... зомби висит в системе\n");
        
        sleep(30);  // Зомби существует 30 секунд
        
        printf("\nРодитель просыпается и завершает работу\n");
        printf("Зомби исчезнет когда завершится родитель\n");
    }
    return 0;
}