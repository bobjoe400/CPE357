#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
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

char** str_to_array(char** arr, char* in_string, char* delim, int arr_size){
	char* temp;

	char** next = arr;
	int counter = 0;
	temp = strtok(in_string, delim);
	while(temp!=NULL && counter < arr_size){
		counter++;
		*next++ = temp;
		temp = strtok(NULL, delim);
	}
	*next = NULL;
	return arr;
}