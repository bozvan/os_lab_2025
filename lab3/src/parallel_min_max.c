#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

pid_t *child_pids = NULL;
int timeout_received = 0;

// Обработчик сигнала SIGALRM
void timeout_handler(int sig) {
    timeout_received = 1;
    printf("Timeout reached! Sending SIGKILL to child processes...\n");
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {
        {"seed", required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {"pnum", required_argument, 0, 0},
        {"by_files", no_argument, 0, 'f'},
        {"timeout", required_argument, 0, 't'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "ft:", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("seed must be a positive number\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("array_size must be a positive number\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              printf("pnum must be a positive number\n");
              return 1;
            }
            break;
          case 3:
            with_files = true;
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;
      case 't':
        timeout = atoi(optarg);
        if (timeout <= 0) {
          printf("timeout must be a positive number\n");
          return 1;
        }
        break;
      case '?':
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"seconds\"]\n",
           argv[0]);
    return 1;
  }

  child_pids = malloc(pnum * sizeof(pid_t));
  if (child_pids == NULL) {
    perror("malloc failed");
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  
  int active_child_processes = 0;
  int pipefd[2];
  int *process_completed = calloc(pnum, sizeof(int)); // Отслеживаем завершенные процессы

  if (!with_files) {
    if (pipe(pipefd) == -1) {
      perror("pipe failed");
      free(array);
      free(child_pids);
      free(process_completed);
      return 1;
    }
  }

  signal(SIGALRM, timeout_handler);

  if (timeout > 0) {
    alarm(timeout);
    printf("Timeout set to %d seconds\n", timeout);
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  // Создаем дочерние процессы
  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      active_child_processes += 1;
      child_pids[i] = child_pid;
      
      if (child_pid == 0) {
        unsigned int chunk_size = array_size / pnum;
        unsigned int start = i * chunk_size;
        unsigned int end = (i == pnum - 1) ? array_size : (i + 1) * chunk_size;
        
        struct MinMax local_min_max = GetMinMax(array, start, end);
        
        if (with_files) {
          char filename[32];
          snprintf(filename, sizeof(filename), "min_max_%d.txt", i);
          FILE *file = fopen(filename, "w");
          if (file == NULL) {
            perror("fopen failed");
            exit(1);
          }
          fprintf(file, "%d %d", local_min_max.min, local_min_max.max);
          fclose(file);
        } else {
          close(pipefd[0]);
          write(pipefd[1], &local_min_max, sizeof(struct MinMax));
          close(pipefd[1]);
        }
        free(array);
        exit(0);
      }

    } else {
      printf("Fork failed!\n");
      free(array);
      free(child_pids);
      free(process_completed);
      return 1;
    }
  }

  if (!with_files) {
    close(pipefd[1]); // Закрываем запись в родительском процессе
  }

  // Ожидаем завершения процессов
  while (active_child_processes > 0) {
    if (timeout_received) {
      printf("Killing remaining child processes...\n");
      for (int i = 0; i < pnum; i++) {
        if (child_pids[i] > 0 && !process_completed[i]) {
          printf("Killing process %d\n", child_pids[i]);
          kill(child_pids[i], SIGKILL);
        }
      }
      break;
    }
    
    int status;
    pid_t finished_pid = waitpid(-1, &status, WNOHANG);
    
    if (finished_pid > 0) {
      active_child_processes -= 1;
      // Отмечаем завершенный процесс
      for (int i = 0; i < pnum; i++) {
        if (child_pids[i] == finished_pid) {
          process_completed[i] = 1;
          printf("Process %d completed successfully\n", finished_pid);
          break;
        }
      }
    } else if (finished_pid == 0) {
      usleep(10000);
    } else {
      perror("waitpid failed");
      break;
    }
  }

  if (timeout > 0) {
    alarm(0);
  }

  // СБОР РЕЗУЛЬТАТОВ
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  int results_collected = 0;
  
  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;
    int result_valid = 0;

    if (with_files) {
      char filename[32];
      snprintf(filename, sizeof(filename), "min_max_%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file != NULL) {
        if (fscanf(file, "%d %d", &min, &max) == 2) {
          result_valid = 1;
          results_collected++;
        }
        fclose(file);
        remove(filename);
      }
    } 
    else {
      if (process_completed[i]) {
        struct MinMax local_min_max;
        if (read(pipefd[0], &local_min_max, sizeof(struct MinMax)) == sizeof(struct MinMax)) {
          min = local_min_max.min;
          max = local_min_max.max;
          result_valid = 1;
          results_collected++;
        }
      }
    }

    if (result_valid) {
      if (min < min_max.min) min_max.min = min;
      if (max > min_max.max) min_max.max = max;
    }
  }

  if (!with_files) {
    close(pipefd[0]);
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);
  free(process_completed);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Processes completed: %d/%d\n", results_collected, pnum);
  if (timeout_received) {
    printf("TIMEOUT: Some processes were terminated\n");
  }
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}