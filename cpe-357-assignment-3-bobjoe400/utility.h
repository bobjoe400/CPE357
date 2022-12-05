#ifndef UTILITY_H
#define UTILITY_H
#include<dirent.h>
#include<stdlib.h>
#include<limits.h>

typedef struct dirent dirent;

typedef struct{
    int show_hidden;
    int show_size;
}FLAGS;

typedef struct{
    char name[256];
    char type;
    int size;
}FileListEntry;

typedef struct{
    int size;
    FileListEntry** entries;
}FileList;

int compare_FLE(const void*, const void*);
void* checked_malloc(size_t);
DIR* checked_opendir(char*);
FileListEntry* FileEntry(char*, char);
FileListEntry* newFileListEntry(char*, char, int);
FileList* addEntry(FileList*, FileListEntry*);
size_t getflistsize(FileList** f_list);
#endif