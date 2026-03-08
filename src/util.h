// src/util.h
#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

void util_seed(unsigned int seed);
int  util_random_int(int min, int max);
float util_random_float(void);
void util_get_timestamp(char *buffer, size_t size);

#endif
