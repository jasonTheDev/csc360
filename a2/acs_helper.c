#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MICRO_PER_SEC           1000000

void safe_gettimeofday(struct timeval *tv) {
    if ((gettimeofday(tv, NULL)) != 0) {
        printf("ERROR: gettimeofday() failed\n");
        exit(1);
    }
}

double diff_seconds(struct timeval *end, struct timeval *start) {
    double diff = difftime(end->tv_sec, start->tv_sec);
    diff += (double) (end->tv_usec - start->tv_usec) / MICRO_PER_SEC;
    return diff;
}

int isDigits(char *string)
{
    if (string == NULL) {
        return 0;
    }

    size_t length = strlen(string);
    long int i = 0;
    while (i < length && isdigit(string[i])) {
        i++;
    }

    return i == length;
}