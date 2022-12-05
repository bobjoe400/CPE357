#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include "utility.h"

#define MAX_CLIENTS 2                        //maximum number of clients
#define BUFF_SIZE 2000                       //size of our read/write buffer
#define MAX_ARGS 64                          //Maximum number of arguments for a cgi-like call

#define NUMELEMS(x) sizeof(x)/sizeof(x[0])   //Used to determine the number of elements in an array

enum HTTP_ERR{ //Used so we are not using magic numbers for our status codes
   OK = 200,
   Bad_Request = 400,
   Permission_Denied = 403,
   Not_Found = 404,
   Internal_Error = 500,
   Not_Implemented = 501
};

enum GET_HEAD{HEAD, GET}; 

int fds[(MAX_CLIENTS)+3];  /*keep track of which pid's have which fd's +4 because 0-2 and 3 is 
                                 used for initial fd in main.*/

int no_proc = 0;           //keep track of the number of clients connected
int has_child = -1;        //flag to keep track of if we ran a command or not

#define USAGE_STRING "usage: %s <desired port between 1024-65535>\n"


//adapted from client.c
int validate_arguments(int argc, char *argv[])
{
   int port = 0;
   if (argc == 0)
   {
      fprintf(stderr, USAGE_STRING, "server");
      exit(EXIT_FAILURE);
   }
   else if (argc < 2 || argc > 2)
   {
      fprintf(stderr, USAGE_STRING, argv[0]);
      exit(EXIT_FAILURE);

   }else if((port = atoi(argv[1]))<=1024||port>65535){
      fprintf(stderr, USAGE_STRING, argv[0]);
      exit(EXIT_FAILURE);
   }
   return port;
}

//given a pid, return the index of that pid in the fds array. This represents the fd being used by the pid
int find_pid(pid_t pid){
   //start at index 4 and iterate through the list of pids to find the fd associated with that pid
   int i = 4; 
   while(i < NUMELEMS(fds)){
      if(fds[i] == pid){
         break;
      }
      i++;
   }
   //check if we reached the end of the list, if so, then we coudln't find the pid
   //else just return the index of the pid
   return (i == NUMELEMS(fds)) ? -1 : i;
}

void handle_child(int signo){
   int status, w, index;
   while((w = waitpid(-1, &status, WNOHANG))>0){
      if((index = find_pid(w))<0){
         fprintf(stderr, "cannot find fd\n");
         exit(1);
      }else{
         close(index);
         printf("SERVER: Process %i using fd %i exited with status %i\n", w, index,  WEXITSTATUS(status));
         printf("SERVER: Connection closed.\n");
         no_proc--;
      }
   }
   return;
}


//runs command and returns the status and file descriptor (if it exists) of the file that the command wrote to
int* run_command(char* command, int* retArr){
   int status, w;
   pid_t pid;

   //convert command into argv array
   char* argv[MAX_ARGS];
   str_to_array(argv, command,  "&?\n", MAX_ARGS);
   
   //create temporary file 
   char* path;
   asprintf(&path, "%i", getpid());
   retArr[1] = open(path, O_CREAT | O_RDWR, S_IRWXU);
   free(path);

   //run command
   retArr[0] = OK;
   if((pid = fork())<0){
      perror("run_command fork");
      retArr[0] = Internal_Error;
   }else if(pid == 0){
      //child
      chdir("cgi-like");
      dup2(retArr[1], STDOUT_FILENO);
      dup2(retArr[1], STDERR_FILENO);
      close(retArr[1]);
      execv(argv[0], argv);
      perror("run_command child");
      exit(1);
   }else{
      //parent
      printf("Process %i: child process %i running command %s\n", getpid(), pid, argv[0]);
      has_child = pid;
      if((w = wait(&status))<0){
         perror("run_command parent");
      }
      if(WIFEXITED(status)){
         //if the child didn't exit normally, then we return an error and remove temp file
         if(WEXITSTATUS(status)!=0){
            char filename[32];
            sprintf(filename, "%i", getpid());
            remove(filename);
            close(retArr[1]);
            retArr[0] = Not_Found;
            retArr[1] = -1;
         }else{
            lseek(retArr[1], 0, SEEK_SET); //reset the file descriptors offset for reading
            printf("Process %i: child process %i exited with status %i\n", getpid(), w, status);
         }
      }
   }
   return retArr;
}

//given a line input, interprets the input, and returns an array of basic data need for the other functions
int* process_request(char* input, int nfd, int retArr[4]){
   
   printf("Process %i: interpreting request\n", getpid());

   retArr[0] = OK;   //status code
   retArr[1] = -1;   //file descriptor
   retArr[2] = 0;    //size of file
   retArr[3] = HEAD; //request type

   //convert input into an array of 3 elements
   char* input_arr[3];
   str_to_array(input_arr, input, " \n", 3);

   //if we have a badly formmated request
   if(input_arr[0] == NULL || input_arr[1] == NULL || input_arr[2] == NULL){
      retArr[0] = Bad_Request;

      printf("Process %i: finished interpreting request\n", getpid());
      return retArr;
   }

   //check for GET or HEAD (or neither)
   if(strcmp(input_arr[0], "GET")== 0){
      retArr[3] = GET;
   }else if (strcmp(input_arr[0], "HEAD") != 0){
      retArr[3] = -1;
      retArr[1] = Bad_Request;

      printf("Process %i: finished interpreting request\n", getpid());
      return retArr;
   }

   //Check if we are trying to use a cgi-like input
   if(strncmp(input_arr[1], "/cgi-like/", 10)==0){
      //since we are only looking in the cgi-like/ folder, we only need the 
      //name of the program thats in that folder and not "/cgi-like/", so we can
      //just advance the pointer to that to point past those chars
      char* cmd = input_arr[1]+10;
      int tmp[2];
      run_command(cmd, tmp);
      retArr[0] = tmp[0];
      retArr[1] = tmp[1];
   }else{ //we try to open the file in the second argument if we didn't run a command
      if(input_arr[1][0] == '/'){
         input_arr[1]++;
      }

      //check for ".." and "./", one is illegal, the other just causes issues
      //if we have them, just ignore them and advance the pointer
      char test[3];
      test[2] = 0;
      strncpy(test, input_arr[1], 2);
      if(strcmp(test, "..") == 0 || strcmp(test, "./")==0){
         input_arr[1]+=2;
      }

      //using open we can see if we can open the file
      //if it was a permissions issue, then set the status to that
      if((retArr[1] = open(input_arr[1], O_RDONLY))==-1){
         retArr[0] = Not_Found;
         if(errno == EACCES){
            retArr[0] = Permission_Denied;
         }
      }
   }

   //if nothing bad has happened we try to run stat
   if(retArr[0] == OK){
      struct stat stats;
      if(fstat(retArr[1], &stats)<0){
         perror("stats");
         retArr[0] = Internal_Error;
      }else{
         retArr[2] = stats.st_size;
      }
   }

   printf("Process %i: finished interpreting request\n", getpid());
   return retArr; //status, fd, size, head/get (0,1)
}

void write_header(int* status, int nfd){
   printf("Process %i: writing header\n", getpid());

   //create the status messsage segment of the header
   char* status_message;
   switch(status[0]){
      case OK:
         status_message = "OK";
         break;
      case Bad_Request:
         status_message = "Bad Request";
         break;
      case Permission_Denied:
         status_message = "Permission Denied";
         break;
      case Not_Found:
         status_message = "Not Found";
         break;
      case Internal_Error:
         status_message = "Internal Error";
         break;
      case Not_Implemented:
         status_message = "Not Implemented";
         break;
   }

   char message[256], error_message[256];

   //the conents of the message need to be an error message, since we don't parse any content if there was an error
   //we can just write it ourselves in this function. However, we need to know the length of that error message, using
   //sprintf() we can get the length of said error message, and write that to the error message char*;
   int len = 0;
   if(status[0] != 200){
      len = sprintf(error_message, "Error %i %s\n", status[0], status_message);
   }

   //build and write the header
   len = sprintf(message, "HTTP/1.0 %i %s\r\nContent-Type: text/html\r\nContent-Length: %i\r\n\r\n", status[0], status_message, (len==0)? status[2] : len);
   write(nfd, message, len);
   printf("Process %i: finished writing header\n", getpid());

   //if there was anything other than okay, just write the error message as the content
   if(status[0] != 200){
      printf("Process %i: writing error message as content\n", getpid());
      write(nfd, error_message, len);
      printf("Process %i: finished writing error message as content\n", getpid());
   }
   return;
}

void parse_content(int nfd, int file){
   printf("Process %i: parsing content from fd %i\n", getpid(), file);
   
   //parse through the content of the file with BUFF_SIZE byte chunks
   int num;
   char buff[BUFF_SIZE];
   while((num = read(file, &buff, BUFF_SIZE)) > 0){
      write(nfd, &buff, BUFF_SIZE);
   }
   printf("Process %i: finished parsing content\n", getpid());

   //if we ran a command, has_child will have been set, we can then delete
   //the temporary file that we created
   if(has_child>0){
      char filename[32];
      sprintf(filename, "%i", getpid());
      remove(filename);
   }
   close(file);
   return;
}


void handle_request(int nfd)
{
   FILE *network = fdopen(nfd, "r");
   char* line = NULL;
   size_t size;
   ssize_t num;

   if (network == NULL)
   {
      perror("fdopen");
      close(nfd);
      return;
   }

   while ((num = getline(&line, &size, network)) >= 0)
   {
      int status[4] = {0}; //[status, fd, size, (GET or HEAD)]
      process_request(line, nfd, status); 
      write_header(status, nfd);
      if(status[0] == OK && status[3] == GET){
         parse_content(nfd, status[1]);
      }
   }
   free(line);
   fclose(network);
}

void run_service(int fd)
{
   //setup our signal handler for our parent
   struct sigaction sa;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_RESTART;
   sa.sa_handler = handle_child;
   sigaction(SIGCHLD, &sa, NULL);

   while (1)
   {
      int nfd = accept_connection(fd);
      if (nfd != -1)
      {
         pid_t pid;
         if((pid = fork())<0){
            perror("run_service");
            exit(1);
         }else if (pid == 0){
            printf("Process %i: Connection established\n", getpid());

            //give the child a new signal handler so we dont trip when we get a
            //SIGCHD when a command is run 
            struct sigaction new_sa;
            sigemptyset(&new_sa.sa_mask);
            new_sa.sa_flags = SA_RESTART;
            new_sa.sa_handler = SIG_DFL;
            sigaction(SIGCHLD, &new_sa, NULL);
            handle_request(nfd);
            exit(0);

         }else{
            
            //we use an array of size 3+MAX_CLIENTS to store the pid of the child clients
            //when a new client connects, it is given an FD. This FD will always be between
            //3 and (3+MAX_CLIENTS). This allows us to close the correct socket when a client
            //disconnects. 
            fds[nfd] = pid;
            printf("SERVER: Process %i established connection using fd %i\n", pid, nfd);
            no_proc++;
            if(no_proc == MAX_CLIENTS){
               printf("SERVER: MAX CLIENTS REACHED\n");
               pause();
            }
         }
      }
   }
}

int main(int argc, char* argv[])
{
   int port = validate_arguments(argc, argv);

   int fd = create_service(port);

   if (fd != -1)
   {
      printf("SERVER: listening on port: %d\n", port);

      run_service(fd);

      close(fd);
   }

   return 0;
}
