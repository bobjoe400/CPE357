#define _GNU_SOURCE
#include<dirent.h>
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<limits.h>
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

int compare_FLE(const void* op1, const void* op2){
    const FileListEntry* fle1 = *(const FileListEntry**) op1;
    const FileListEntry* fle2 = *(const FileListEntry**) op2;

    //correctly sorting hidden files
    return(strcasecmp((fle1->name[0] == '.')? fle1->name+1 : fle1->name, 
                      (fle2->name[0] == '.')? fle2->name+1 : fle2->name));
}

DIR* checked_opendir(char* requested_dir){
    DIR* dir = NULL;
    if(!(dir = opendir(requested_dir))){
        fprintf(stderr, "tt: '%s' %s\n", requested_dir, strerror(errno));
    }
    return dir;
}

FileListEntry* newFileListEntry(char* name, char type, int size){
	FileListEntry* newEntry = checked_malloc(sizeof(FileListEntry));
    strcpy(newEntry->name, name);
    newEntry->type = type;
    newEntry->size = size;
    return newEntry;
}

FileList* addEntry(FileList* f_list, FileListEntry* newEntry){
    f_list->entries = (FileListEntry**) realloc(f_list->entries, (sizeof(FileListEntry*) * (f_list->size+1)));
    f_list->entries[f_list->size] = newEntry;
    f_list->size = f_list->size +1;
    return f_list;
}

size_t getflistsize(FileList** f_list){
	return (*f_list)->size;
}