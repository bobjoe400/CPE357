#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <search.h>
#include "utility.h"

void* checked_malloc(size_t size){
	void *p;
	p = malloc(size);
	if(p == NULL){
		perror("malloc");
		exit(1);
	}

	return p;
}

FILE* checked_fopen(char* file, char* arg){
	FILE* ptr_f = fopen(file, arg);
	if(ptr_f == NULL){
        fprintf(stderr, "Error opening file: %s\n", file);
        return ptr_f;
    }
	return ptr_f;
}

char* uint32_to_str(uint32_t i){
   int length = snprintf(NULL, 0, "%lu", (unsigned long)i);       // pretend to print to a string to get length
   char* str = checked_malloc(length + 1);                        // allocate space for the actual string
   snprintf(str, length + 1, "%lu", (unsigned long)i);            // print to string

   return str;
}

//code for this basically verbatim from the lecture;
char* trimwhitespace(char* str){
	//leading
	size_t idx;
	for(idx = 0; str[idx]!='\0' && isspace(str[idx]); idx++){}
	str = &str[idx];

	//trailing
	size_t size = strlen(str);
	while(isspace(str[size - 1])){
		str[size-1] = '\0';
		size--;
	}
	return str;
}