#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "pman_helper.h"

/*
 * Tokenizes string via given deliminators and asigns
 * each token to the given list
 * Precondition: list_length must be 1 or greater
 * Returns: 0 for success, -1 otherwise
 */
int tokenize(char *string, char **list, const char *delims, int list_length) {
    int index = 0;
    char *ptr;

    ptr = strtok(string, delims);
    if(ptr == NULL) {
        return -1;
    }
    list[index] = ptr;
    index++;

    while(ptr != NULL && index < list_length) {
        ptr = strtok(NULL, delims);
        list[index] = ptr;
        index++;
    }
    return 0;
}