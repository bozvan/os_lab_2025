#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "common.h"

struct Server {
  char ip[255];
  int port;
};

bool ReadServersFromFile(const char *filename, struct Server **servers, int *servers_num) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Cannot open servers file: %s\n", filename);
    return false;
  }

  *servers_num = 0;
  char line[255];
  while (fgets(line, sizeof(line), file)) {
    (*servers_num)++;
  }

  *servers = malloc(sizeof(struct Server) * (*servers_num));
  if (!*servers) {
    fclose(file);
    return false;
  }

  rewind(file);
  int i = 0;
  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = 0;
    
    char *colon = strchr(line, ':');
    if (!colon) {
      fprintf(stderr, "Invalid server format: %s\n", line);
      fclose(file);
      free(*servers);
      return false;
    }
    
    *colon = '\0';
    strncpy((*servers)[i].ip, line, sizeof((*servers)[i].ip) - 1);
    (*servers)[i].port = atoi(colon + 1);
    i++;
  }

  fclose(file);
  return true;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers_file[255] = {'\0'};

  while (true) {

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        break;
      case 2:
        strncpy(servers_file, optarg, sizeof(servers_file) - 1);
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == UINT64_MAX || mod == UINT64_MAX || !strlen(servers_file)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  
  struct Server *to = NULL;
  int servers_num = 0;
  if (!ReadServersFromFile(servers_file, &to, &servers_num)) {
    fprintf(stderr, "Failed to read servers from file\n");
    return 1;
  }

  uint64_t total = 1;
  int active_servers = 0;
  
  for (int i = 0; i < servers_num; i++) {
    struct hostent *hostname = gethostbyname(to[i].ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      continue;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(to[i].port);
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      continue;
    }

    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
      fprintf(stderr, "Connection failed to %s:%d\n", to[i].ip, to[i].port);
      close(sck);
      continue;
    }

    uint64_t begin = (k * i) / servers_num + 1;
    uint64_t end = (k * (i + 1)) / servers_num;
    
    if (begin > end) {
      begin = end;
    }

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
      fprintf(stderr, "Send failed to %s:%d\n", to[i].ip, to[i].port);
      close(sck);
      continue;
    }

    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Receive failed from %s:%d\n", to[i].ip, to[i].port);
      close(sck);
      continue;
    }

    uint64_t partial_result = 0;
    memcpy(&partial_result, response, sizeof(uint64_t));
    total = MultModulo(total, partial_result, mod);
    active_servers++;
    
    close(sck);
  }

  if (active_servers == 0) {
    fprintf(stderr, "No servers responded\n");
    free(to);
    return 1;
  }

  printf("%"PRIu64"! mod %"PRIu64" = %"PRIu64"\n", k, mod, total);
  free(to);

  return 0;
}