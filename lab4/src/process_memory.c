/* Program to display address information about the process */
/* Adapted from Gray, J., program 1.4 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>

/* Below is a macro definition */
#define SHW_ADR(ID, I) (printf("ID %s \t is at virtual address: %8lX\n", ID, (unsigned long)&I))

extern int etext, edata, end; /* Global variables for process memory */

char *cptr = "This message is output by the function showit()\n"; /* Static */
char buffer1[25];

int showit(char *p); /* Function prototype */

int main() {  // Добавили возвращаемый тип int
  int i = 0; /* Automatic variable */

  /* Printing addressing information */
  printf("\nAddress etext: %8lX \n", (unsigned long)&etext);
  printf("Address edata: %8lX \n", (unsigned long)&edata);
  printf("Address end  : %8lX \n", (unsigned long)&end);

  SHW_ADR("main", main);
  SHW_ADR("showit", showit);
  SHW_ADR("cptr", cptr);
  SHW_ADR("buffer1", buffer1);
  SHW_ADR("i", i);
  
  strcpy(buffer1, "A demonstration\n");   /* Library function */
  write(1, buffer1, strlen(buffer1) + 1); /* System call */
  showit(cptr);

  return 0; // Добавили возврат значения
} /* end of main function */

/* A function follows */
int showit(char *p) {  // Исправили объявление функции
  char *buffer2;
  SHW_ADR("buffer2", buffer2);
  
  if ((buffer2 = (char *)malloc((unsigned)(strlen(p) + 1))) != NULL) {
    printf("Allocated memory at %lX\n", (unsigned long)buffer2);
    strcpy(buffer2, p);    /* copy the string */
    printf("%s", buffer2); /* Display the string */
    free(buffer2);         /* Release location */
  } else {
    printf("Allocation error\n");
    exit(1);
  }
  
  return 0; // Добавили возврат значения
}