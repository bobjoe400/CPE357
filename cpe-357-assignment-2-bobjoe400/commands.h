#include "utility.h"

#ifndef COMMAND_H
#define COMMAND_H
int find_inode(int*, char*, char*);
int command(char*, char*, int*);
void list(char*, int*);
void makefiledir(char*, int*, char);
void chgdir(char*, int*, char*);
#endif