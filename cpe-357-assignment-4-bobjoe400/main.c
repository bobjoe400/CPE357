#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <search.h>
#include "utility.h"

int createQueue(Queue* queue, FILE* ptr_f){
	//intialize values for parsing downloads file
	int line_no = 0;
	char* line = NULL;
	char* pEnd = NULL;
	size_t len = 0;
	ssize_t nread;
	int error = 0;

	/*
	There are three cases of error when it comes to downloads file formatting. In order of checking, 
	they are:
		- only one argument
		- time argument not a valid int
		- there are more than 3 arguments
	
	*/

	//parse download list
	while((nread = getline(&line,&len, ptr_f)) != -1){
		line_no+=1;
		char* token = strtok(line, " ");
		char* filename = strdup(token);

		//skip over empty lines
		if(filename[0] == '\n'){
			free(filename);
			continue;
		}

		token = strtok(NULL, " ");
		if(token == NULL){
			fprintf(stderr, "dl: Improperly formatted downloads file. Incorrect line: %i\n",line_no);
			error = 1;
			free(filename);
			continue;
		}
		char* url = strdup(token);
		
		//trim the newline if only 2 arguments
		url[strcspn(url, "\n")] = 0;
		token = strtok(NULL, " ");
		char* waittime = NULL;

		//trim the newline from the 3rd arguement (if it exists)
		if(token != NULL){
			waittime = strdup(token);
			waittime[strcspn(waittime, "\n")] = 0;
			int check = strtol(waittime, &pEnd, 0);
			if((check < 0 || pEnd[0] != '\0')){
				fprintf(stderr, "dl: Improperly formatted downloads file. Incorrect line: %i\n",line_no);
				error = 1;
				free(filename);
				free(url);
				free(waittime);
				continue;
			}
		}
		if((strtok(NULL," ") != NULL)){
			fprintf(stderr, "dl: Improperly formatted downloads file. Incorrect line: %i\n",line_no);
			free(filename);
			free(url);
			free(waittime);
			error = 1;
			continue;
		}
		
		//add download to queue
		enqueue(queue,url,filename, waittime);
	}
	free(line);
	fclose(ptr_f);

	return error;
}

void spawnProcesses(Queue* queue, int MAX_PROCESS){
	//we use a linked list to keep track of processes for easy memory usage
	LinkedList* processes = (LinkedList*) checked_malloc(sizeof(LinkedList));

	//initialize value for spawning processes
	processes->head = NULL;
	int proc_run = 0;
	int line_no = 0;

	//spawn dowload processes
	//while the below statement is essentially true, this is another layer of protection incase we somehow get
	//out of control while spawning processes
	while(proc_run != MAX_PROCESS){
		proc_run += 1;
		line_no +=1;

		pid_t dlp_pid, w;
		int status;
		
		//get the parameters of the current process to spawn
		Node* childProcParam = dequeue(queue);
		if((dlp_pid=fork())<0){
			perror("dl");
			exit(1);	
		}else if(dlp_pid == 0){
			printf("process %i starting with processing line #%i\n", getpid(), line_no);

			//There are two options for curl, either waittime exists or it doesn't
			if(childProcParam->waittime == NULL){	
				execlp("curl", "curl", "-o", childProcParam->filename, "-s", childProcParam->url, (char * ) NULL);
				perror("	dl");
				exit(1);
			}else{
				execlp("curl", "curl", "-m",childProcParam->waittime, "-o", childProcParam->filename, "-s", childProcParam->url, (char * ) NULL);
				perror("	dl");
				exit(1);
			}
		}else{

			//add the spawned process to our process list
			addListElem(processes, dlp_pid, line_no);

			/*
			There are three states of the process spawner. In order they are:
				- We have spawned the max number of processes and we need to wait
				- We have no more downloads to process, wait on all the processes to finish
				- We check if a process has finished before starting another
			
			In each condition the process after waiting is the same.
				1. We removed the completed process from the process list
				2. We print the status of that process
				3. Free the memory that was allocated for the removed item
			
			Finally, we need to free the memory created by the removed queue item.
			*/
			if(proc_run == MAX_PROCESS){
				w = wait(&status);
				print_exit(w, status, listDelete(processes,w));
				proc_run--;
			}if(isQueueEmpty(queue)){
				while((w = wait(&status))> 0){
					print_exit(w, status, listDelete(processes, w));
					
				}
				freeQueueElem(childProcParam);
				break;
			}if((w = waitpid(-1, &status, WNOHANG)) > 0){
				print_exit(w, status, listDelete(processes,w));
				proc_run--;
			}
			freeQueueElem(childProcParam);
		}
	}

	//free our linked list and our queue
	free(processes);
	free(queue);
	return;
}

int main(int argc, char* argv[]){

	//check if we entered in the right arguments
	if(argc != 3){
		fprintf(stderr, "dl: 'dl' takes 2 arguments, 'file name' and 'max processes'\n");
		exit(1);
	}

	//try to open the file with our provided file name
	FILE* ptr_f;
	char* file_to_open = argv[1];
	if((ptr_f = fopen(file_to_open,"r"))==NULL){
		perror("dl");
		exit(1);
	}

	//this is used to see if we've entered in an positive int
	char* pEnd = NULL;
	int MAX_PROCESS = strtol(argv[2], &pEnd, 0);

	if(MAX_PROCESS <=0  || pEnd[0] != '\0'){
		fprintf(stderr, "dl: Invalid process max\n");
		fclose(ptr_f);
		exit(1);
	}

	//add all the downloads we need to our queue from reading the file
	Queue* queue = (Queue *)checked_malloc(sizeof(Queue));
	queue->front = NULL;
	queue->rear = NULL;

	//if there was an error parsing the downloads file, just exit the program
	if(createQueue(queue, ptr_f) == 1){
		free(queue);
		exit(1);
	}

	//empty file (not an error)
	if(isQueueEmpty(queue)){
		free(queue);
		printf("dl: '%s' is empty\n", argv[1]);
		return(0);
	}

	//with queue created and parsed, start spawning processes 
	spawnProcesses(queue, MAX_PROCESS);

	return(0);
}