#ifndef _ACSHELPER_H_
#define _ACSHELPER_H_

void safe_gettimeofday(struct timeval *tv);
double diff_seconds(struct timeval *end, struct timeval *start);
int isDigits(char *string);

#endif
