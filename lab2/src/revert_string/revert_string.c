#include "revert_string.h"
#include <string.h>
#include <stdlib.h>

void RevertString(char *str) {
    int length = strlen(str);
    char *reversed = (char*)malloc(length + 1);
    
	for (int i = 0; i < length; i++) {
        reversed[i] = str[length - 1 - i];
    }
    reversed[length] = '\0';

    strcpy(str, reversed);
    free(reversed);
}