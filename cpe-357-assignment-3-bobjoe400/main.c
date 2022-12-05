#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "utility.h"

void list(char* dir_to_open, int offset, FLAGS* flags){

    //check if directory exists or we have permission to enter
    if(chdir(dir_to_open)==-1){
        fprintf(stderr, "tt: '%s' %s\n", dir_to_open, strerror(errno));
        return;
    }

    //if we can enter the directory, but not read it
    DIR* dir = NULL;
    if((dir = checked_opendir("./")) == NULL){
        chdir(".."); //needed for when we chdir into directory and need to go back up again
        return;
    };

    //generate the tab offset for pretty-printing
    char* taboffset = checked_malloc(sizeof(char)*3*offset+1);
    memset(taboffset, '\0', sizeof(char)*3*offset+1);
    for(int i=0; i<offset; i++){
        strcat(taboffset, "   ");
    }
    
    //initialize subdirs list
    FileList* subdirs = (FileList*) checked_malloc(sizeof(FileList));
    subdirs->entries = NULL;
    subdirs->size = 0;
    
    dirent* currEntry = NULL;
    while((currEntry = readdir(dir))!=NULL){

        //dont process if the entry name is '.' or '..'
        //we have to use strings for *both* as a hidden file will ring true for the first check
        if(!(strcmp(currEntry->d_name,".")==0 || strcmp(currEntry->d_name,"..")==0)){
            
            //use stat to get filesize
            struct stat* stats = checked_malloc(sizeof(struct stat));
            memset(stats, '\0', sizeof(struct stat)); //makes valgrind happy

            lstat(currEntry->d_name, stats);
            int fsize = 0;
            fsize = stats->st_size;
            free(stats);//dont need stat anymore

            /* Truth table for below:
              '.' show_hidden | ~'.' | (!('.') v show_hidden)
              ----------------|------|-----------------------
                1      1      |   0  |          1
                1      0      |   0  |          0
                0      1      |   1  |          1
                0      0      |   1  |          1
                
            This produces the correct output as the only time we don't
            want it to print is when we dont want to show hidden and the
            file starts with a '.'  */
            if(!(currEntry->d_name[0] == '.') || flags->show_hidden){
                //create a new entry and add it to the list of subdirs
                FileListEntry* newEntry = newFileListEntry(currEntry->d_name, currEntry->d_type, fsize);
                subdirs = addEntry(subdirs, newEntry);
            }
        }
    }
    closedir(dir);
    //since we only have 1 dir open per level, we can set the number of open dirs to 1
    //and not have to worry about anything being upset


    //sort the subdirs
    qsort(subdirs->entries,subdirs->size, sizeof(FileListEntry*),compare_FLE);

    //iterate through the subdirs, recursively calling calling the function if we get
    //to a directory
    for(int i = 0; i<subdirs->size; i++){
        printf("%s--| %s", taboffset,subdirs->entries[i]->name);
        if(flags->show_size){
            printf(" [size: %i]\n",subdirs->entries[i]->size);
        }else{
            printf("\n");
        }
        if(subdirs->entries[i]->type == DT_DIR) list(subdirs->entries[i]->name, offset+1, flags);
    }
    //iterate through the subdirs entries, freeing them
    for(int i=0; i<subdirs->size; i++){
        free(subdirs->entries[i]);
    }
    free(subdirs->entries);
    free(subdirs);
    free(taboffset);
    chdir(".."); //needed for when we chdir into directory and need to go back up again
    return;
}

int main(int argc, char* argv[]){
    //create and initialized our struct for the hidden and size flags
    FLAGS* flags = checked_malloc(sizeof(FLAGS));
    flags->show_hidden = 0;
    flags->show_size = 0;

    //getopt is very useful for processing input
    int opt;
    while((opt = getopt(argc, argv, "as")) != -1){
        switch(opt){
            case 'a':
                flags->show_hidden = 1;
                break;
            case 's':
                flags->show_size = 1;
                break;
            default:
                fprintf(stderr, "Invalid flags. Only \"-a\" and \"-s\" are allowed.\n");
                exit(EXIT_FAILURE);
        }
    }

    char* o_cwd = getcwd(NULL, 0);
    int index = 0;

    //see if we passed directories into the argument list
    while(argv[optind+index] != NULL){
        printf("\n");
        if(chdir(argv[optind+index])==-1){
            fprintf(stderr, "tt: '%s' %s\n", argv[optind+index], strerror(errno));
            index++;
            continue;
        }

        //generate first name to print off using cwd 
        char* cwd = getcwd(NULL, 0);
        char* dir = strrchr(cwd, '/'); //print off the name of the cwd
        printf("%s", dir+1); //+1 to get rid of the "/"


        //get stats for cwd so we can print off if flag is set
        struct stat* stats = checked_malloc(sizeof(struct stat));
        memset(stats, '\0', sizeof(struct stat)); //makes valgrind happy

        lstat(".", stats);
        int fsize = 0;
        fsize = stats->st_size;
        free(stats);

        (flags->show_size)?printf(" [size: %i]\n",fsize):printf("\n");

        //lets start recursing!
        //list(flags, req_dir, 0);
        list( "./", 0, flags);
        free(cwd);
        chdir(o_cwd);
        index++;
    }
    if(index == 0){
        //generate first name to print off using cwd 
        char* cwd = getcwd(NULL, 0);
        char* dir = strrchr(cwd, '/'); //print off the name of the cwd
        printf("%s", dir+1); //+1 to get rid of the "/"


        //get stats for cwd so we can print off if flag is set
        struct stat* stats = checked_malloc(sizeof(struct stat));
        memset(stats, '\0', sizeof(struct stat)); //makes valgrind happy

        lstat(".", stats);
        int fsize = 0;
        fsize = stats->st_size;
        free(stats);

        (flags->show_size)?printf(" [size: %i]\n",fsize):printf("\n");

        //lets start recursing!
        //list(flags, req_dir, 0);
        list( "./", 0, flags);
        free(cwd);
    }
    //free resources and exit
    free(o_cwd);
    free(flags);
    exit(EXIT_SUCCESS);
}