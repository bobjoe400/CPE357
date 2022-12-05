#ifndef UTILITY_H
#define UTILITY_H
#include<stdio.h>

typedef struct Node Node;
typedef struct ListElem ListElem;

struct Node{
	char* filename;
	char* url;
	char* waittime;
	Node* next;
};

struct ListElem{
	int line_no;
	int pid;
	ListElem* next;
};

typedef struct{
	Node* front;
	Node* rear;
}Queue;

typedef struct{
	ListElem* head;
}LinkedList;

void* checked_malloc(size_t);
void print_exit(pid_t, int, ListElem*);

void enqueue(Queue*, char*, char*, char*);
Node* dequeue(Queue*);
int isQueueEmpty(Queue*);
void freeQueueElem(Node*);

void addListElem(LinkedList*, int, int);
ListElem* listDelete(LinkedList*, int);
int isListEmpty(LinkedList*);

#endif