#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <search.h>
#include "utility.h"

int find_inode(int *curr_inode, char* inode_list, char* dir){
    //opens the guaranteed directory file
    char* inode_num = uint32_to_str(*curr_inode);
    FILE* ptr_f = NULL;
    if((ptr_f = checked_fopen(inode_num,"r"))==NULL){
        free(inode_num);
        return -2;
    }
    free(inode_num);
    char* name = checked_malloc(sizeof(char)*33);//33 so we can still use strcmp if we have a 
                                                 //non-null-character-terminated entry
    memset(name, '\0', 33);
    uint32_t* inode = checked_malloc(sizeof(uint32_t));
    int rval = -1; //default to not found
    fseek(ptr_f, 0, SEEK_END);
    int len = ftell(ptr_f); //length of file to determine end of reading
    rewind(ptr_f);
    while(ftell(ptr_f) != len){
        fread(inode, sizeof(uint32_t), 1, ptr_f);
        fread(name, sizeof(char), 32, ptr_f);
        if(strcmp(name, dir) == 0){ //if name in directory file = dir name we have found the file
            rval = *inode;          //and return the inode
            fclose(ptr_f);
            free(name);
            free(inode);
            return rval;
        }
    }
    fclose(ptr_f);
    free(name);
    free(inode);
    return rval;
}

void list(char* inode_list, int* curr_inode){
    //open the file guaranteed directory file at our inode
    //guaranteed becase we start at inode 0 (directory) and can only
    //cd into other directories
    char* inode_num = uint32_to_str(*curr_inode);
    FILE* ptr_f = NULL;
    if((ptr_f = checked_fopen(inode_num,"r"))==NULL){
        free(inode_num);
        return;
    }
    free(inode_num);
    char* name = checked_malloc(sizeof(char)*33);
    memset(name, '\0', sizeof(char)*33);
    uint32_t inode;
    fseek(ptr_f, 0, SEEK_END);
    int len = ftell(ptr_f);//length of file to determine when to stop reading
    rewind(ptr_f);
    while(ftell(ptr_f) != len){
        fread(&inode, sizeof(uint32_t), 1, ptr_f);
        printf("%i ", inode);
        fread(name, sizeof(char), 32, ptr_f);
        printf("%s\n", name);
    }
    free(name);
    fclose(ptr_f);
}

void makefiledir(char* inode_list, int* curr_inode, char* dir, char* mode){
    //if the user game mkdir or touch an operand 
    if(dir == NULL){
        fprintf(stderr, "%s: missing %soperand\n", (strcmp(mode, "d")==0)? "mkdir" : "touch", (strcmp(mode, "d")==0)? "" : "file ");
        return;
    }

    //check if we have space or if the directory exists
    if(strlen(inode_list) == 1024){
        fprintf(stderr, ((strcmp(mode,"d")==0) ? "mkdir: No space available\n" : "touch: No space available\n"));
        return;
    }
    int node;
    if((node = find_inode(curr_inode, inode_list, dir)) > -1){
        if(strcmp(mode,"d")==0){
            fprintf(stderr, "mkdir: cannot create directory \'%s\': File exists\n", dir);
            return;
        }else{
            return;
        }
    }else if(node == -2){
        return;
    }

    //add filename to curr_inode file (guaranteed to be directory)
    char* inode_num = uint32_to_str(*curr_inode);
    FILE* ptr_f = NULL;
    if((ptr_f = checked_fopen(inode_num,"a"))==NULL){
        free(inode_num);
        return;
    }
    free(inode_num);
    uint32_t* new_inode = checked_malloc(sizeof(uint32_t));
    *new_inode = strlen(inode_list);
    fwrite(new_inode, sizeof(uint32_t), 1, ptr_f);
    fwrite(dir, sizeof(char), 32, ptr_f);
    fclose(ptr_f);

    //create actual inode file
    inode_num = uint32_to_str(*new_inode);
    if((ptr_f = checked_fopen(inode_num,"w+"))==NULL){
        free(inode_num);
        return;
    }
    free(inode_num);
    if(strcmp(mode,"d")==0){
        char* name = ".";
        fwrite(new_inode, sizeof(uint32_t), 1, ptr_f);
        fwrite(name,sizeof(char),32,ptr_f);
        fwrite((uint32_t*)curr_inode, sizeof(uint32_t), 1,ptr_f);
        name = "..";
        fwrite(name, sizeof(char),32,ptr_f);
    }else{
        fwrite(dir, sizeof(char),32,ptr_f);
    }
    fclose(ptr_f);

    //add filetype to inodes_list file (updating the inode_list array comes later back in main)
    if((ptr_f = checked_fopen("inodes_list","a"))==NULL){
        free(new_inode);
        return;
    }
    fwrite(new_inode, sizeof(uint32_t), 1, ptr_f);
    fwrite(mode, sizeof(char),1,ptr_f);
    fclose(ptr_f);

    free(new_inode);
}

void chgdir(int* curr_inode, char* inode_list, char* dir){
    //check if the user gave cd an operand
    if(dir == NULL){
        fprintf(stderr, "cd: Please specify a directory\n");
        return;
    }
    //try to find the directory in the inode_list and let the user know if the directory exists
    int found_item = find_inode(curr_inode, inode_list, dir);
    if(found_item < 0){
        fprintf(stderr, "cd: %s: No such file or directory\n",dir);
    }else if(inode_list[found_item] != 'd'){
        fprintf(stderr, "cd: %s: Not a directory\n",dir);
    }else{
        *curr_inode = found_item;
    }
}

int command(char* cmd, char* inode_list, int* curr_inode){
    //break command up (only worry about command and 1st argument)
    char* d_cmd = strtok(cmd, " ");
    char* i_arg = strtok(NULL, " ");
    //because i_arg is of undetermined size and we need to eventually write (at most) 32 bytes of i_arg,
    //we need to declare a new pointer and (if i_arg populated) copy the value of i_arg into it so we do
    //not run into fwrite trying to access uninitialized addresses.
    char* arg = checked_malloc(sizeof(char)*33); //make 33 to keep strcmp working in case of a 32 char name
    memset(arg,'\0',33);
    if(i_arg != NULL){
        strncpy(arg, i_arg,32);
    }
    //set return value default to 0
    int rtval = 0;
    if(strcmp(d_cmd, "exit") == 0){
        rtval = 1;
    }else if(strcmp(d_cmd, "ls") == 0){
        list(inode_list, curr_inode);
    }else if(strcmp(d_cmd, "cd") == 0){
        chgdir(curr_inode, inode_list, arg);
    }else if(strcmp(d_cmd, "mkdir") == 0){
        makefiledir(inode_list, curr_inode, arg, "d");
        rtval = 2;  //sets return value to 2, indicating the need to update the inode_list array
    }else if(strcmp(d_cmd,"touch") == 0){
        makefiledir(inode_list, curr_inode, arg, "f");
        rtval = 2;
    }else{
        rtval = -1;
    }
    free(arg);
    return(rtval);
}