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

void print_exit(pid_t pid, int status, ListElem* removed){
	
	//either the process exited normally or it didn't
	if(status == 0){
		printf("process %i processing line #%i exited normally\n", pid, removed->line_no);
	}else{
		printf("process %i processing line #%i exited abnormally with status %i\n", pid, removed->line_no, WEXITSTATUS(status));
	}
	free(removed);
}


void enqueue(Queue* queue, char* url, char* filename, char* waittime){

	//create entry
	Node* newNode = checked_malloc(sizeof(Node));
	newNode->filename = filename;
	newNode->url = url;
	newNode->waittime = waittime;
	newNode->next = NULL;
	
	//if empty queue both are set to newNode
	if(isQueueEmpty(queue)){
		queue->front = newNode;
		queue->rear = newNode;
	}else{
		queue->rear->next = newNode;
		queue->rear = newNode;
	}
	return;
}

Node* dequeue(Queue* queue){
	if(queue->front == NULL){
		fprintf(stderr, "Queue is empty\n");
		return NULL;
	}

	//allocate space for the return node and copy the front into the return value
	Node* retNode = checked_malloc(sizeof(Node));
	memcpy(retNode, queue->front, sizeof(Node));

	//remove and set the front node
	free(queue->front);
	queue->front = retNode->next;

	//if the queue is empty don't forget to set the rear to NULL too
	if(isQueueEmpty(queue)){
		queue->rear = NULL;
	}
	return retNode;
}

int isQueueEmpty(Queue* queue){
	return queue->front == NULL;
}

void freeQueueElem(Node* node){
	//queue has strdup'd char*'s so we need to free them individually
	free(node->filename);
	free(node->url);
	free(node->waittime);
	free(node);
	return;
}

void addListElem(LinkedList* list, int pid, int line_no){

	//initialize new element
	ListElem* newElem = (ListElem*) checked_malloc(sizeof(ListElem));
	newElem->line_no = line_no;
	newElem->pid = pid;
	newElem->next = NULL;

	if(isListEmpty(list)){
		list->head = newElem;
		return;
	}

	//find end of list and add element
	ListElem* current = list->head;
	while(current->next != NULL){
		current = current->next;
	}
	current->next = newElem;
	return;
}

ListElem* listDelete(LinkedList* list, int pid){
	struct ListElem* temp = list->head, *prev;
	struct ListElem* retVal = (ListElem*) checked_malloc(sizeof(ListElem));
    
	//if first elements contains the data
    if (temp != NULL && temp->pid == pid) {
		memcpy(retVal, temp, sizeof(ListElem));
        list->head = temp->next; 
        free(temp); 
        return retVal;
    }
	
	//in the middle or end
    while (temp != NULL && temp->pid != pid) {
        prev = temp;
        temp = temp->next;
    }
	
	//return if not found (shouldn't happen)
    if (temp == NULL) return NULL;
	
	//copy data into the return value, unlink the nodes and free the unlinked node and return
	memcpy(retVal, temp, sizeof(ListElem));
    prev->next = temp->next;
	free(temp);
	return retVal;
}

int isListEmpty(LinkedList* list){
	return list->head == NULL;
}