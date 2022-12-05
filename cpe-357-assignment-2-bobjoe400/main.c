#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <stdbool.h>
#include "utility.h"
#include "commands.h"

void generate_inode_list(char** inode_list, FILE* ptr_f){
	//frees inode_list pointer and re-mallocs the memory space
	free(*inode_list);
	*inode_list = (char*) checked_malloc(sizeof(char)*1025);
	memset(*inode_list, '\0', 1025);
	uint32_t num;
	char type;
	size_t len;
	fseek(ptr_f, 0, SEEK_END);
	len = ftell(ptr_f); //length of file to determine end of reading inodes_list
	rewind(ptr_f);
	while(ftell(ptr_f) != len){
		fread(&num, sizeof(uint32_t),1,ptr_f);
		fread(&type, sizeof(char), 1, ptr_f);
		(*inode_list)[num] = type;
	}
}

int main(int argc, char *argv[]){
	//opening directory from argument
	char *fs;
	if(argc == 2){
		fs = argv[1];
	}else{
		fprintf(stderr,"Please assign a starting directory in the arguments");
		exit(1);
	}
	if(chdir(fs) != 0){
		fprintf(stderr,"Not a directory");
		exit(1);
	}
	FILE *ptr_f;
	if(!(ptr_f = fopen("inodes_list","r"))){
		fprintf(stderr, "inodes_list not found");
		exit(1);
	}

	//initializing inode_list
	char* inode_list = checked_malloc(sizeof(char)*1025);
	//inode_list[1024] = '\0'; //setting size to 1025 chars and setting last char to null
	memset(inode_list, '\0',1025);
	int curr_inode = 0;		 //allows us to use strlen to determine next inode to populate
	generate_inode_list(&inode_list, ptr_f);
	fclose(ptr_f);

	//checking if inode 0 is a directory
	if(inode_list[curr_inode] != 'd'){
		fprintf(stderr, "inode 0 is not a directory");
		exit(1);
	}

	//reading user input
	char* input = NULL;
	size_t len = 0;
	size_t bytes_read = 0;
	bool run = true;
	do{
		printf(">");
		if((bytes_read = getline(&input, &len, stdin))== -1){ //if the users presses EOF
			printf("\nexiting...\n");
			break;
		}
		trimwhitespace(input);
		switch(command(input, inode_list, &curr_inode)){
			case -1:
				fprintf(stderr, "Invalid command\n");
				break;
			case 1:
				printf("exiting...\n");
				run = false;
				break;
			case 2:	//if the users calles mkdir or touch we need to remake the inode_list
				ptr_f = fopen("inodes_list", "r");
				generate_inode_list(&inode_list, ptr_f);
				fclose(ptr_f);
				break;
			default:
				break;	
		}
	}while(run);
	free(input);
	free(inode_list);
	return(0);
}