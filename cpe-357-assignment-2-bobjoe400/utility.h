#include <stdint.h>
#include <stdlib.h>

#ifndef UTILITY_H
#define UTILITY_H
void* checked_malloc(size_t);
FILE* checked_fopen(char*,char*);
char* uint32_to_str(uint32_t);
char* trimwhitespace(char*);
#endif